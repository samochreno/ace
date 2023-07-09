#include "Emitter.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <filesystem>
#include <fstream>
#include <climits>
#include <cinttypes>

#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Target/TargetMachine.h>

#include "Scope.hpp"
#include "Symbols/All.hpp"
#include "BoundNodes/All.hpp"
#include "Log.hpp"
#include "Assert.hpp"
#include "DynamicCastFilter.hpp"
#include "Name.hpp"
#include "SpecialIdentifier.hpp"
#include "Core.hpp"
#include "Compilation.hpp"

namespace Ace
{
    LabelBlockMap::LabelBlockMap(
        Emitter& t_emitter
    ) : m_Emitter{ t_emitter }
    {
    }

    auto LabelBlockMap::GetOrCreateAt(
        const LabelSymbol* const t_labelSymbol
    ) -> llvm::BasicBlock*
    {
        const auto matchingBlockIt = m_Map.find(t_labelSymbol);
        if (matchingBlockIt != end(m_Map))
        {
            return matchingBlockIt->second;
        }

        auto block = llvm::BasicBlock::Create(
            *m_Emitter.GetCompilation()->LLVMContext,
            "",
            m_Emitter.GetFunction()
        );

        m_Map[t_labelSymbol] = block;
        return block;
    }

    auto LabelBlockMap::Clear() -> void
    {
        m_Map.clear();
    }

    Emitter::Emitter(
        const Compilation* const t_compilation
    ) : m_Compilation{ t_compilation },
        m_Module
        {
            std::make_unique<llvm::Module>(
                "module",
                *t_compilation->LLVMContext
            )
        },
        m_LabelBlockMap{ *this }
    {
        m_Module->setTargetTriple(
            llvm::sys::getProcessTriple()
        );
    }

    Emitter::~Emitter()
    {
    }

    auto Emitter::SetASTs(
        const std::vector<std::shared_ptr<const ModuleBoundNode>>& t_asts
    ) -> void
    {
        m_ASTs = t_asts;
    }

    auto Emitter::Emit() -> Result
    {
        auto* const globalScope = m_Compilation->GlobalScope.Unwrap().get();
        const auto& packageName = m_Compilation->Package.Name;
        const auto& natives     = m_Compilation->Natives;

        m_C.Initialize(*m_Compilation->LLVMContext, *m_Module);

        const auto& now = std::chrono::steady_clock::now;

        const auto timeIREmittingStart = now();

        const auto symbols = globalScope->CollectAllDefinedSymbolsRecursive();

        const auto allTypeSymbols = DynamicCastFilter<ITypeSymbol*>(symbols);
        std::vector<ITypeSymbol*> typeSymbols{};
        std::copy_if(
            begin(allTypeSymbols),
            end  (allTypeSymbols),
            back_inserter(typeSymbols),
            [](ITypeSymbol* const t_typeSymbol)
            {
                return !t_typeSymbol->IsTemplatePlaceholder();
            }
        );

        const auto structSymbols   = DynamicCastFilter<StructTypeSymbol*>(typeSymbols);
        const auto variableSymbols = DynamicCastFilter<StaticVarSymbol*>(symbols);

        std::vector<const IBoundNode*> nodes{}; 
        std::for_each(begin(m_ASTs), end(m_ASTs),
        [&](const std::shared_ptr<const ModuleBoundNode>& t_ast)
        {
            const auto children = t_ast->GetChildren();
            nodes.insert(end(nodes), begin(children), end(children));
        });
        const auto functionNodes = DynamicCastFilter<const FunctionBoundNode*>(nodes);
       
        EmitNativeTypes();
        EmitStructTypes(structSymbols);

        EmitStaticVars(variableSymbols);

        const auto allFunctionSymbols = globalScope->CollectSymbolsRecursive<FunctionSymbol>();
        std::vector<FunctionSymbol*> functionSymbols{};
        std::copy_if(
            begin(allFunctionSymbols),
            end  (allFunctionSymbols),
            back_inserter(functionSymbols),
            [](FunctionSymbol* const t_functionSymbol)
            {
                return !t_functionSymbol->IsTemplatePlaceholder();
            }
        );
        EmitFunctions(functionSymbols);
        
        std::vector<const FunctionSymbol*> mainFunctionSymbols{};
        std::for_each(begin(functionNodes), end(functionNodes),
        [&](const FunctionBoundNode* const t_functionNode)
        {
            if (
                t_functionNode->GetSymbol()->GetName() != 
                SpecialIdentifier::Main
                ) 
                return;

            mainFunctionSymbols.push_back(t_functionNode->GetSymbol());
        });

        ACE_ASSERT(mainFunctionSymbols.size() == 1);
        auto* const mainFunctionSymbol = mainFunctionSymbols.front();
        ACE_ASSERT(
            mainFunctionSymbol->GetType()->GetUnaliased() == 
            natives->Int.GetSymbol()
        );
        ACE_ASSERT(
            mainFunctionSymbol->GetSymbolCategory() ==
            SymbolCategory::Static
        );
        ACE_ASSERT(mainFunctionSymbol->CollectAllParams().empty());

        auto* const mainType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*m_Compilation->LLVMContext),
            false
        );

        auto* const mainFunction = llvm::Function::Create(
            mainType,
            llvm::Function::ExternalLinkage,
            "main",
            *m_Module
        );

        auto* const mainBlock = llvm::BasicBlock::Create(
            *m_Compilation->LLVMContext,
            "",
            mainFunction
        );

        llvm::IRBuilder<> mainBuilder{ mainBlock };
        auto* const mainValue = mainBuilder.CreateCall(
            m_FunctionMap.at(mainFunctionSymbols.front())
        );
        mainBuilder.CreateRet(mainValue);

        std::string originalModuleString{};
        llvm::raw_string_ostream originalModuleOStream{ originalModuleString };
        originalModuleOStream << *m_Module;
        originalModuleOStream.flush();
        originalModuleString.clear();

        const auto timeIREmittingEnd = now();

        const auto timeAnalysesStart = now();

        llvm::LoopAnalysisManager lam{};
        llvm::FunctionAnalysisManager fam{};
        llvm::CGSCCAnalysisManager cgam{};
        llvm::ModuleAnalysisManager mam{};

        llvm::PassBuilder pb{};

        pb.registerModuleAnalyses(mam);
        pb.registerCGSCCAnalyses(cgam);
        pb.registerFunctionAnalyses(fam);
        pb.registerLoopAnalyses(lam);
        pb.crossRegisterProxies(lam, fam, cgam, mam);
        
        auto mpm = pb.buildPerModuleDefaultPipeline(
            llvm::OptimizationLevel::O3
        );
        mpm.run(*m_Module, mam);

        const auto timeAnalysesEnd = now();

        std::string moduleString{};
        llvm::raw_string_ostream moduleOStream{ moduleString };
        moduleOStream << *m_Module;
        moduleOStream.flush();
        std::ofstream irFileStream
        { 
            m_Compilation->OutputPath / (packageName + ".ll") 
        };
        ACE_ASSERT(irFileStream);
        irFileStream << moduleString;
        moduleString.clear();

        std::error_code errorCode{};
        llvm::raw_fd_ostream bitcodeFileOStream
        { 
            (m_Compilation->OutputPath / (packageName + ".bc")).string(),
            errorCode, 
            llvm::sys::fs::OF_None 
        };
        ACE_ASSERT(!errorCode);
        llvm::WriteBitcodeToFile(*m_Module, bitcodeFileOStream);
        bitcodeFileOStream.close();
        
        const auto timeLLCStart = now();

        const std::string llc = 
            "llc -O3 -opaque-pointers -relocation-model=pic -filetype=obj "
            "-o " + (m_Compilation->OutputPath / (packageName + ".obj")).string() + " " +
            (m_Compilation->OutputPath / (packageName + ".bc")).string();
        system(llc.c_str());

        const auto timeLLCEnd = now();

        const auto timeClangStart = now();

        const std::string clang = 
            "clang -lc -lm "
            "-o " + (m_Compilation->OutputPath / packageName).string() + " " +
            (m_Compilation->OutputPath / (packageName + ".obj")).string();
        system(clang.c_str());

        const auto timeClangEnd = now();

        return Result
        {
            Result::DurationInfo
            {
                timeIREmittingEnd - timeIREmittingStart,
                timeAnalysesEnd - timeAnalysesStart,
                timeLLCEnd - timeLLCStart,
                timeClangEnd - timeClangStart
            }
        };
    }

    auto Emitter::EmitFunctionBodyStmts(
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& t_stmts
    ) -> void
    {
        const auto paramSymbols = m_FunctionSymbol->CollectAllParams();
        for (size_t i = 0; i < paramSymbols.size(); i++)
        {
            auto* const paramSymbol = paramSymbols.at(i);
            auto* const typeSymbol = paramSymbol->GetType();

            auto* const allocaInst = m_BlockBuilder->Builder.CreateAlloca(
                GetIRType(typeSymbol),
                nullptr,
                paramSymbol->GetName()
            );

            EmitCopy(
                allocaInst,
                m_Function->arg_begin() + i,
                typeSymbol
            );

            m_LocalVarMap[paramSymbol] = allocaInst;
        }

        for (size_t i = 0; i < t_stmts.size(); i++)
        {
            const auto* const stmt = t_stmts.at(i).get();

            ACE_ASSERT(!m_StmtIndexMap.contains(stmt)); 
            m_StmtIndexMap[stmt] = i;
        
            auto* const variableStmt = 
                dynamic_cast<const VarStmtBoundNode*>(stmt);

            if (variableStmt)
            {
                auto* const variableSymbol = variableStmt->GetSymbol();
                ACE_ASSERT(!m_LocalVarSymbolStmtIndexMap.contains(
                    variableSymbol
                ));
                m_LocalVarSymbolStmtIndexMap[variableSymbol] = i;
                m_LocalVarSymbolStmtIndexPairs.push_back(
                    LocalVarSymbolStmtIndexPair{ variableSymbol, i }
                );
            }
        }

        std::for_each(
            begin(m_LocalVarSymbolStmtIndexPairs), 
            end(m_LocalVarSymbolStmtIndexPairs), 
            [&](const LocalVarSymbolStmtIndexPair& t_symbolIndexPair)
            {
                auto* const variableSymbol = t_symbolIndexPair.LocalVarSymbol;
                auto* const type = GetIRType(variableSymbol->GetType());
                ACE_ASSERT(!m_LocalVarMap.contains(variableSymbol));
                m_LocalVarMap[variableSymbol] = m_BlockBuilder->Builder.CreateAlloca(
                    type,
                    nullptr,
                    variableSymbol->GetName()
                );
            }
        );

        const auto blockStartIndicies = [&]() -> std::vector<size_t>
        {
            std::vector<size_t> indicies{};

            if (!t_stmts.empty())
            {
                indicies.push_back(0);
            }

            for (size_t i = 0; i < t_stmts.size(); i++)
            {
                auto* const stmt = t_stmts.at(i).get();
                auto* const labelStmt =
                    dynamic_cast<const LabelStmtBoundNode*>(stmt);

                if (labelStmt)
                {
                    indicies.push_back(i);
                }
            }

            return indicies;
        }();

        for (size_t i = 0; i < blockStartIndicies.size(); i++)
        {
            const bool isLastBlock = i == (blockStartIndicies.size() - 1);

            const auto beginStmtIt = begin(t_stmts) + blockStartIndicies.at(i);
            const auto endStmtIt   = isLastBlock ?
                                          end  (t_stmts) :
                                          begin(t_stmts) + blockStartIndicies.at(i + 1);

            std::for_each(beginStmtIt, endStmtIt,
            [&](const std::shared_ptr<const IStmtBoundNode>& t_stmt)
            {
                if (t_stmt.get() == beginStmtIt->get())
                {
                    auto* const labelStmt =
                        dynamic_cast<const LabelStmtBoundNode*>(t_stmt.get());

                    if (labelStmt)
                    {
                        auto* const block = GetLabelBlockMap().GetOrCreateAt(
                            labelStmt->GetLabelSymbol()
                        );

                        auto blockBuilder = std::make_unique<BlockBuilder>(block);

                        if (
                            m_BlockBuilder->Block->empty() ||
                            !m_BlockBuilder->Block->back().isTerminator()
                            )
                        {
                            m_BlockBuilder->Builder.CreateBr(blockBuilder->Block);
                        }

                        SetBlockBuilder(std::move(blockBuilder));
                    }
                }

                const bool isTerminated = 
                    !m_BlockBuilder->Block->empty() && 
                    m_BlockBuilder->Block->back().isTerminator();

                if (isTerminated)
                    return;

                t_stmt->Emit(*this);
                
                auto* const blockEndStmt = 
                    dynamic_cast<const BlockEndStmtBoundNode*>(t_stmt.get());

                if (blockEndStmt)
                {
                    const auto blockScope = blockEndStmt->GetSelfScope();
                    auto blockVarSymbols = 
                        blockScope->CollectSymbols<LocalVarSymbol>();

                    std::sort(
                        begin(blockVarSymbols),
                        end  (blockVarSymbols),
                        [&](
                            const LocalVarSymbol* const t_lhs,
                            const LocalVarSymbol* const t_rhs
                            )
                        {
                            return
                                m_LocalVarSymbolStmtIndexMap.at(t_lhs) >
                                m_LocalVarSymbolStmtIndexMap.at(t_rhs);
                        }
                    );

                    std::for_each(
                        begin(blockVarSymbols),
                        end  (blockVarSymbols),
                        [&](LocalVarSymbol* const t_variableSymbol)
                        {
                            EmitDrop({ 
                                m_LocalVarMap.at(t_variableSymbol), 
                                t_variableSymbol->GetType() 
                            });
                        }
                    );
                }
            });
        }

        if (m_Function->getReturnType()->isVoidTy())
        {
            if (
                m_BlockBuilder->Block->empty() ||
                !m_BlockBuilder->Block->back().isTerminator()
                )
            {
                EmitDropArgs();

                m_BlockBuilder->Builder.CreateRetVoid();
            }
        }
    }

    auto Emitter::EmitLoadArg(
        const size_t t_index, 
        llvm::Type* const t_type
    ) const -> llvm::Value*
    {
        return m_BlockBuilder->Builder.CreateLoad(
            t_type,
            m_Function->arg_begin() + t_index
        );
    }

    auto Emitter::EmitCopy(
        llvm::Value* const t_lhsValue, 
        llvm::Value* const t_rhsValue, 
        ITypeSymbol* const t_typeSymbol
    ) -> void
    {
        auto* const type = GetIRType(t_typeSymbol);
        auto* const ptrType = llvm::PointerType::get(type, 0);

        if (t_typeSymbol->IsReference())
        {
            auto* const loadInst = m_BlockBuilder->Builder.CreateLoad(
                type,
                t_rhsValue
            );

            m_BlockBuilder->Builder.CreateStore(
                loadInst,
                t_lhsValue
            );

            return;
        }

        auto* const glueSymbol = t_typeSymbol->GetCopyGlue().value();

        auto* const lhsAllocaInst = m_BlockBuilder->Builder.CreateAlloca(ptrType);
        m_BlockBuilder->Builder.CreateStore(t_lhsValue, lhsAllocaInst);

        auto* const rhsAllocaInst = m_BlockBuilder->Builder.CreateAlloca(ptrType);
        m_BlockBuilder->Builder.CreateStore(t_rhsValue, rhsAllocaInst);

        m_BlockBuilder->Builder.CreateCall(
            m_FunctionMap.at(glueSymbol),
            { lhsAllocaInst, rhsAllocaInst }
        );
    }

    auto Emitter::EmitDrop(const ExprDropData& t_dropData) -> void
    {
        if (t_dropData.TypeSymbol->IsReference())
            return;

        auto operatorName = t_dropData.TypeSymbol->CreateFullyQualifiedName();
        operatorName.Sections.push_back(
            SymbolNameSection{ SpecialIdentifier::Operator::Drop }
        );

        const auto glueSymbol = t_dropData.TypeSymbol->GetDropGlue().value();

        auto* const typeSymbol = glueSymbol->CollectParams().front()->GetType();
        auto* const type = GetIRType(typeSymbol);

        auto* const allocaInst = m_BlockBuilder->Builder.CreateAlloca(type);
        m_BlockBuilder->Builder.CreateStore(t_dropData.Value, allocaInst);

        m_BlockBuilder->Builder.CreateCall(
            m_FunctionMap.at(glueSymbol),
            { allocaInst }
        );
    }

    auto Emitter::EmitDropTemporaries(
        const std::vector<ExprDropData>& t_temporaries
    ) -> void
    {
        std::for_each(rbegin(t_temporaries), rend(t_temporaries),
        [&](const ExprDropData& t_temporary)
        {
            EmitDrop(t_temporary);
        });
    }

    auto Emitter::EmitDropLocalVarsBeforeStmt(
        const IStmtBoundNode* const t_stmt
    ) -> void
    {
        const auto stmtIndex = m_StmtIndexMap.at(t_stmt);
        auto scope = t_stmt->GetScope();

        std::for_each(
            rbegin(m_LocalVarSymbolStmtIndexPairs), 
            rend(m_LocalVarSymbolStmtIndexPairs),
            [&](const LocalVarSymbolStmtIndexPair& t_symbolIndexPair)
            {
                const auto variableStmtIndex =
                    t_symbolIndexPair.StmtIndex;

                if (variableStmtIndex >= stmtIndex)
                {
                    return;
                }

                auto* const variableSymbol =
                    t_symbolIndexPair.LocalVarSymbol;

                if (variableSymbol->GetScope() != scope)
                {
                    if (
                        variableSymbol->GetScope()->GetNestLevel() >=
                        scope->GetNestLevel()
                        )
                    {
                        return;
                    }

                    scope = variableSymbol->GetScope();
                }

                EmitDrop({
                    m_LocalVarMap.at(variableSymbol),
                    variableSymbol->GetType()
                });
            }
        );

        EmitDropArgs();
    }

    auto Emitter::EmitDropArgs() -> void
    {
        const auto paramSymbols = m_FunctionSymbol->CollectAllParams();
        std::for_each(rbegin(paramSymbols), rend(paramSymbols),
        [&](IParamVarSymbol* const t_paramSymbol)
        {
            EmitDrop({
                m_LocalVarMap.at(t_paramSymbol),
                t_paramSymbol->GetType()
            });
        });
    }

    auto Emitter::GetCompilation() const -> const Compilation*
    {
        return m_Compilation;
    }

    auto Emitter::GetModule() const -> llvm::Module&
    {
        return *m_Module.get();
    }

    auto Emitter::GetC() const -> const C&
    {
        return m_C;
    }

    auto Emitter::GetIRType(
        const ITypeSymbol* const t_typeSymbol
    ) const -> llvm::Type*
    {
        auto* const pureTypeSymbol =
            t_typeSymbol->GetWithoutReference()->GetUnaliased();
        auto* const pureType = m_TypeMap.at(pureTypeSymbol);

        const bool isReference = t_typeSymbol->IsReference();
        
        return isReference ?
            llvm::PointerType::get(pureType, 0) :
            pureType;
    }

    auto Emitter::GetStaticVarMap() const -> const std::unordered_map<const StaticVarSymbol*, llvm::Constant*>&
    {
        return m_StaticVarMap;
    }

    auto Emitter::GetFunctionMap() const -> const std::unordered_map<const FunctionSymbol*, llvm::FunctionCallee>&
    {
        return m_FunctionMap;
    }

    auto Emitter::GetLocalVarMap() const -> const std::unordered_map<const IVarSymbol*, llvm::Value*>&
    {
        return m_LocalVarMap;
    }

    auto Emitter::GetLabelBlockMap() -> LabelBlockMap&
    {
        return m_LabelBlockMap;
    }

    auto Emitter::GetFunction() const -> llvm::Function*
    {
        return m_Function;
    }

    auto Emitter::GetBlockBuilder() -> BlockBuilder&
    {
        return *m_BlockBuilder.get();
    }

    auto Emitter::SetBlockBuilder(
        std::unique_ptr<BlockBuilder>&& t_value
    ) -> void
    {
        m_BlockBuilder = std::move(t_value);
    }

    auto Emitter::EmitNativeTypes() -> void
    {
        for (auto& typeSymbolPair : m_Compilation->Natives->GetIRTypeSymbolMap())
        {
            m_TypeMap[typeSymbolPair.first] = typeSymbolPair.second;
        }
    }

    auto Emitter::EmitStructTypes(
        const std::vector<StructTypeSymbol*>& t_structSymbols
    ) -> void
    {
        std::for_each(begin(t_structSymbols), end(t_structSymbols),
        [&](const StructTypeSymbol* const t_structSymbol)
        {
            if (t_structSymbol->IsPrimitivelyEmittable())
                return;

            m_TypeMap[t_structSymbol] = llvm::StructType::create(
                *m_Compilation->LLVMContext,
                t_structSymbol->CreateSignature()
            );
        });

        std::for_each(begin(t_structSymbols), end(t_structSymbols),
        [&](const StructTypeSymbol* const t_structSymbol)
        {
            if (t_structSymbol->IsPrimitivelyEmittable())
                return;

            std::vector<llvm::Type*> elements{};

            const auto variableSymbols = t_structSymbol->GetVars();
            std::for_each(begin(variableSymbols), end(variableSymbols),
            [&](const InstanceVarSymbol* const t_variableSymbol)
            {
                auto* const type = GetIRType(t_variableSymbol->GetType());
                elements.push_back(type);
            });

            auto* const structType = m_TypeMap.at(t_structSymbol);
            static_cast<llvm::StructType*>(structType)->setBody(elements);
        });
    }

    auto Emitter::EmitStaticVars(
        const std::vector<StaticVarSymbol*>& t_variableSymbols
    ) -> void
    {
        std::for_each(begin(t_variableSymbols), end(t_variableSymbols),
        [&](const StaticVarSymbol* const t_variableSymbol)
        {
            const auto name = t_variableSymbol->CreateSignature();
            auto* const type = GetIRType(t_variableSymbol->GetType());

            m_Module->getOrInsertGlobal(
                name,
                llvm::PointerType::get(type, 0)
            );

            auto* const variable = m_Module->getNamedGlobal(name);
            variable->setInitializer(llvm::Constant::getNullValue(variable->getType()));
            variable->setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);

            m_StaticVarMap[t_variableSymbol] = variable;
        });
    }

    auto Emitter::EmitFunctions(
        const std::vector<FunctionSymbol*>& t_functionSymbols
    ) -> void
    {
        struct FunctionSymbolBlockPair
        {
            llvm::Function* Function{};
            FunctionSymbol* Symbol{};
            llvm::BasicBlock* Block{};
        };

        std::vector<FunctionSymbolBlockPair> functionsSymbolBlockPairs{};
        std::transform(
            begin(t_functionSymbols), 
            end  (t_functionSymbols), 
            back_inserter(functionsSymbolBlockPairs), 
            [&](FunctionSymbol* const t_functionSymbol)
            {
                std::vector<llvm::Type*> paramTypes{};
                const auto paramSymbols =
                    t_functionSymbol->CollectAllParams();
                std::transform(
                    begin(paramSymbols), 
                    end  (paramSymbols), 
                    back_inserter(paramTypes), 
                    [&](IParamVarSymbol* const t_paramSymbol)
                    {
                        return llvm::PointerType::get(
                            GetIRType(t_paramSymbol->GetType()), 
                            0
                        );
                    }
                );

                auto* const type = llvm::FunctionType::get(
                    GetIRType(t_functionSymbol->GetType()),
                    paramTypes,
                    false
                );

                auto* const function = llvm::Function::Create(
                    type,
                    llvm::Function::ExternalLinkage,
                    t_functionSymbol->CreateSignature(),
                    *m_Module
                );

                m_FunctionMap[t_functionSymbol] = function;

                auto* const block = llvm::BasicBlock::Create(
                    *m_Compilation->LLVMContext,
                    "",
                    function
                );

                return FunctionSymbolBlockPair
                { 
                    function,
                    t_functionSymbol,
                    block
                };
            }
        );

        auto clearFunctionData = [&]() -> void
        {
            m_Function = nullptr;
            m_FunctionSymbol = nullptr;
            m_BlockBuilder = nullptr;

            m_LocalVarMap.clear();
            m_LabelBlockMap.Clear();
            m_StmtIndexMap.clear();
            m_LocalVarSymbolStmtIndexMap.clear();
            m_LocalVarSymbolStmtIndexPairs.clear();
        };

        std::for_each(
            begin(functionsSymbolBlockPairs),
            end  (functionsSymbolBlockPairs),
            [&](FunctionSymbolBlockPair& t_functionSymbolBlockPair)
            {
                clearFunctionData();

                m_Function = t_functionSymbolBlockPair.Function;
                m_FunctionSymbol = t_functionSymbolBlockPair.Symbol;
                m_BlockBuilder = std::make_unique<BlockBuilder>(
                    t_functionSymbolBlockPair.Block
                );

                ACE_ASSERT(m_FunctionSymbol->GetBody().has_value());
                t_functionSymbolBlockPair.Symbol->GetBody().value()->Emit(*this);

                const bool isBodyTerminated =
                    m_BlockBuilder->Block->back().isTerminator();

                if (!isBodyTerminated)
                {
                    m_BlockBuilder->Builder.CreateUnreachable();
                }
            }
        );

        clearFunctionData();
    }
}
