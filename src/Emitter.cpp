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
#include "SpecialIdent.hpp"
#include "Compilation.hpp"

namespace Ace
{
    LabelBlockMap::LabelBlockMap(
        Emitter& emitter
    ) : m_Emitter{ emitter }
    {
    }

    auto LabelBlockMap::GetOrCreateAt(
        const LabelSymbol* const labelSymbol
    ) -> llvm::BasicBlock*
    {
        const auto matchingBlockIt = m_Map.find(labelSymbol);
        if (matchingBlockIt != end(m_Map))
        {
            return matchingBlockIt->second;
        }

        auto block = llvm::BasicBlock::Create(
            *m_Emitter.GetCompilation()->LLVMContext,
            "",
            m_Emitter.GetFunction()
        );

        m_Map[labelSymbol] = block;
        return block;
    }

    auto LabelBlockMap::Clear() -> void
    {
        m_Map.clear();
    }

    Emitter::Emitter(
        const Compilation* const compilation
    ) : m_Compilation{ compilation },
        m_Module
        {
            std::make_unique<llvm::Module>(
                "module",
                *compilation->LLVMContext
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
        const std::vector<std::shared_ptr<const ModuleBoundNode>>& asts
    ) -> void
    {
        m_ASTs = asts;
    }

    static auto SaveModuleToFile(
        const Compilation* const compilation,
        const llvm::Module* const module,
        const char* const extension
    ) -> void
    {
        const std::filesystem::path filePath = 
            compilation->OutputPath /
            (compilation->Package.Name + "." + extension);

        std::string moduleString{};

        llvm::raw_string_ostream moduleOStream{ moduleString };
        moduleOStream << *module;
        moduleOStream.flush();

        std::ofstream irFileStream{ filePath };
        ACE_ASSERT(irFileStream);
        irFileStream << moduleString;

        moduleString.clear();
    }

    auto Emitter::Emit() -> Result
    {
        auto* const globalScope = m_Compilation->GlobalScope.Unwrap().get();
        const auto& packageName = m_Compilation->Package.Name;
        const auto& natives     = m_Compilation->Natives;

        m_C.Initialize(*m_Compilation->LLVMContext, *m_Module);

        const auto& now = std::chrono::steady_clock::now;

        const auto timeIREmittingBegin = now();

        const auto symbols = globalScope->CollectAllDefinedSymbolsRecursive();

        const auto allTypeSymbols = DynamicCastFilter<ITypeSymbol*>(symbols);
        std::vector<ITypeSymbol*> typeSymbols{};
        std::copy_if(
            begin(allTypeSymbols),
            end  (allTypeSymbols),
            back_inserter(typeSymbols),
            [](ITypeSymbol* const typeSymbol)
            {
                return !typeSymbol->IsTemplatePlaceholder();
            }
        );

        const auto structSymbols = DynamicCastFilter<StructTypeSymbol*>(typeSymbols);
        const auto varSymbols    = DynamicCastFilter<StaticVarSymbol*>(symbols);

        std::vector<const IBoundNode*> nodes{}; 
        std::for_each(begin(m_ASTs), end(m_ASTs),
        [&](const std::shared_ptr<const ModuleBoundNode>& ast)
        {
            const auto children = ast->CollectChildren();
            nodes.insert(end(nodes), begin(children), end(children));
        });
        const auto functionNodes = DynamicCastFilter<const FunctionBoundNode*>(nodes);
       
        EmitNativeTypes();
        EmitStructTypes(structSymbols);

        EmitStaticVars(varSymbols);

        const auto allFunctionSymbols = globalScope->CollectSymbolsRecursive<FunctionSymbol>();
        std::vector<FunctionSymbol*> functionSymbols{};
        std::copy_if(
            begin(allFunctionSymbols),
            end  (allFunctionSymbols),
            back_inserter(functionSymbols),
            [](FunctionSymbol* const functionSymbol)
            {
                return !functionSymbol->IsTemplatePlaceholder();
            }
        );
        EmitFunctions(functionSymbols);
        
        std::vector<const FunctionSymbol*> mainFunctionSymbols{};
        std::for_each(begin(functionNodes), end(functionNodes),
        [&](const FunctionBoundNode* const functionNode)
        {
            if (
                functionNode->GetSymbol()->GetName().String != 
                SpecialIdent::Main
                ) 
                return;

            mainFunctionSymbols.push_back(functionNode->GetSymbol());
        });

        ACE_ASSERT(mainFunctionSymbols.size() == 1);
        auto* const mainFunctionSymbol = mainFunctionSymbols.front();
        ACE_ASSERT(
            mainFunctionSymbol->GetType()->GetUnaliased() == 
            natives->Int.GetSymbol()
        );
        ACE_ASSERT(mainFunctionSymbol->GetCategory() == SymbolCategory::Static);
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

        // TODO: Make this optional with a CLI option
        SaveModuleToFile(
            m_Compilation,
            m_Module.get(),
            "ll"
        );

        const auto timeIREmittingEnd = now();

        const auto timeAnalysesBegin = now();

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

        // TODO: Make this optional with a CLI option
        SaveModuleToFile(
            m_Compilation,
            m_Module.get(),
            "opt.ll"
        );

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
        
        const auto timeLLCBegin = now();

        const std::string llc = 
            "llc -O3 -opaque-pointers -relocation-model=pic -filetype=obj "
            "-o " + (m_Compilation->OutputPath / (packageName + ".obj")).string() + " " +
            (m_Compilation->OutputPath / (packageName + ".bc")).string();
        system(llc.c_str());

        const auto timeLLCEnd = now();

        const auto timeClangBegin = now();

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
                timeIREmittingEnd - timeIREmittingBegin,
                timeAnalysesEnd - timeAnalysesBegin,
                timeLLCEnd - timeLLCBegin,
                timeClangEnd - timeClangBegin,
            }
        };
    }

    auto Emitter::EmitFunctionBodyStmts(
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
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
                paramSymbol->GetName().String
            );

            EmitCopy(
                allocaInst,
                m_Function->arg_begin() + i,
                typeSymbol
            );

            m_LocalVarMap[paramSymbol] = allocaInst;
        }

        for (size_t i = 0; i < stmts.size(); i++)
        {
            const auto* const stmt = stmts.at(i).get();

            ACE_ASSERT(!m_StmtIndexMap.contains(stmt)); 
            m_StmtIndexMap[stmt] = i;
        
            auto* const varStmt = dynamic_cast<const VarStmtBoundNode*>(stmt);

            if (varStmt)
            {
                auto* const varSymbol = varStmt->GetSymbol();
                ACE_ASSERT(!m_LocalVarSymbolStmtIndexMap.contains(varSymbol));
                m_LocalVarSymbolStmtIndexMap[varSymbol] = i;
                m_LocalVarSymbolStmtIndexPairs.push_back(
                    LocalVarSymbolStmtIndexPair{ varSymbol, i }
                );
            }
        }

        std::for_each(
            begin(m_LocalVarSymbolStmtIndexPairs), 
            end(m_LocalVarSymbolStmtIndexPairs), 
            [&](const LocalVarSymbolStmtIndexPair& symbolIndexPair)
            {
                auto* const varSymbol = symbolIndexPair.LocalVarSymbol;
                auto* const type = GetIRType(varSymbol->GetType());
                ACE_ASSERT(!m_LocalVarMap.contains(varSymbol));
                m_LocalVarMap[varSymbol] = m_BlockBuilder->Builder.CreateAlloca(
                    type,
                    nullptr,
                    varSymbol->GetName().String
                );
            }
        );

        const auto blockBeginIndicies = [&]() -> std::vector<size_t>
        {
            std::vector<size_t> indicies{};

            if (!stmts.empty())
            {
                indicies.push_back(0);
            }

            for (size_t i = 0; i < stmts.size(); i++)
            {
                auto* const stmt = stmts.at(i).get();
                auto* const labelStmt =
                    dynamic_cast<const LabelStmtBoundNode*>(stmt);

                if (labelStmt)
                {
                    indicies.push_back(i);
                }
            }

            return indicies;
        }();

        for (size_t i = 0; i < blockBeginIndicies.size(); i++)
        {
            const bool isLastBlock = i == (blockBeginIndicies.size() - 1);

            const auto beginStmtIt = begin(stmts) + blockBeginIndicies.at(i);
            const auto endStmtIt   = isLastBlock ?
                                          end  (stmts) :
                                          begin(stmts) + blockBeginIndicies.at(i + 1);

            std::for_each(beginStmtIt, endStmtIt,
            [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
            {
                if (stmt.get() == beginStmtIt->get())
                {
                    auto* const labelStmt =
                        dynamic_cast<const LabelStmtBoundNode*>(stmt.get());

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

                stmt->Emit(*this);
                
                auto* const blockEndStmt = 
                    dynamic_cast<const BlockEndStmtBoundNode*>(stmt.get());

                if (blockEndStmt)
                {
                    const auto blockScope = blockEndStmt->GetSelfScope();
                    auto blockVarSymbols = 
                        blockScope->CollectSymbols<LocalVarSymbol>();

                    std::sort(
                        begin(blockVarSymbols),
                        end  (blockVarSymbols),
                        [&](
                            const LocalVarSymbol* const lhs,
                            const LocalVarSymbol* const rhs
                            )
                        {
                            return
                                m_LocalVarSymbolStmtIndexMap.at(lhs) >
                                m_LocalVarSymbolStmtIndexMap.at(rhs);
                        }
                    );

                    std::for_each(
                        begin(blockVarSymbols),
                        end  (blockVarSymbols),
                        [&](LocalVarSymbol* const varSymbol)
                        {
                            EmitDrop({ 
                                m_LocalVarMap.at(varSymbol), 
                                varSymbol->GetType() 
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
        const size_t index, 
        llvm::Type* const type
    ) const -> llvm::Value*
    {
        return m_BlockBuilder->Builder.CreateLoad(
            type,
            m_Function->arg_begin() + index
        );
    }

    auto Emitter::EmitCopy(
        llvm::Value* const lhsValue, 
        llvm::Value* const rhsValue, 
        ITypeSymbol* const typeSymbol
    ) -> void
    {
        auto* const type = GetIRType(typeSymbol);
        auto* const ptrType = llvm::PointerType::get(type, 0);

        if (typeSymbol->IsRef())
        {
            auto* const loadInst = m_BlockBuilder->Builder.CreateLoad(
                type,
                rhsValue
            );

            m_BlockBuilder->Builder.CreateStore(
                loadInst,
                lhsValue
            );

            return;
        }

        auto* const glueSymbol = typeSymbol->GetCopyGlue().value();

        auto* const lhsAllocaInst = m_BlockBuilder->Builder.CreateAlloca(ptrType);
        m_BlockBuilder->Builder.CreateStore(lhsValue, lhsAllocaInst);

        auto* const rhsAllocaInst = m_BlockBuilder->Builder.CreateAlloca(ptrType);
        m_BlockBuilder->Builder.CreateStore(rhsValue, rhsAllocaInst);

        m_BlockBuilder->Builder.CreateCall(
            m_FunctionMap.at(glueSymbol),
            { lhsAllocaInst, rhsAllocaInst }
        );
    }

    auto Emitter::EmitDrop(const ExprDropData& dropData) -> void
    {
        if (dropData.TypeSymbol->IsRef())
        {
            return;
        }

        const auto glueSymbol = dropData.TypeSymbol->GetDropGlue().value();

        auto* const typeSymbol = glueSymbol->CollectParams().front()->GetType();
        auto* const type = GetIRType(typeSymbol);

        auto* const allocaInst = m_BlockBuilder->Builder.CreateAlloca(type);
        m_BlockBuilder->Builder.CreateStore(dropData.Value, allocaInst);

        m_BlockBuilder->Builder.CreateCall(
            m_FunctionMap.at(glueSymbol),
            { allocaInst }
        );
    }

    auto Emitter::EmitDropTmps(
        const std::vector<ExprDropData>& tmps
    ) -> void
    {
        std::for_each(rbegin(tmps), rend(tmps),
        [&](const ExprDropData& temp)
        {
            EmitDrop(temp);
        });
    }

    auto Emitter::EmitDropLocalVarsBeforeStmt(
        const IStmtBoundNode* const stmt
    ) -> void
    {
        const auto stmtIndex = m_StmtIndexMap.at(stmt);
        auto scope = stmt->GetScope();

        std::for_each(
            rbegin(m_LocalVarSymbolStmtIndexPairs), 
            rend(m_LocalVarSymbolStmtIndexPairs),
            [&](const LocalVarSymbolStmtIndexPair& symbolIndexPair)
            {
                const auto varStmtIndex = symbolIndexPair.StmtIndex;
                if (varStmtIndex >= stmtIndex)
                {
                    return;
                }

                auto* const varSymbol = symbolIndexPair.LocalVarSymbol;

                if (varSymbol->GetScope() != scope)
                {
                    if (
                        varSymbol->GetScope()->GetNestLevel() >=
                        scope->GetNestLevel()
                        )
                    {
                        return;
                    }

                    scope = varSymbol->GetScope();
                }

                EmitDrop({
                    m_LocalVarMap.at(varSymbol),
                    varSymbol->GetType()
                });
            }
        );

        EmitDropArgs();
    }

    auto Emitter::EmitDropArgs() -> void
    {
        const auto paramSymbols = m_FunctionSymbol->CollectAllParams();
        std::for_each(rbegin(paramSymbols), rend(paramSymbols),
        [&](IParamVarSymbol* const paramSymbol)
        {
            EmitDrop({
                m_LocalVarMap.at(paramSymbol),
                paramSymbol->GetType()
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
        const ITypeSymbol* const typeSymbol
    ) const -> llvm::Type*
    {
        auto* const pureTypeSymbol =
            typeSymbol->GetWithoutRef()->GetUnaliased();
        auto* const pureType = m_TypeMap.at(pureTypeSymbol);

        const bool isRef = typeSymbol->IsRef();
        
        return isRef ?
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
        std::unique_ptr<BlockBuilder>&& value
    ) -> void
    {
        m_BlockBuilder = std::move(value);
    }

    auto Emitter::EmitNativeTypes() -> void
    {
        for (auto& typeSymbolPair : m_Compilation->Natives->GetIRTypeSymbolMap())
        {
            m_TypeMap[typeSymbolPair.first] = typeSymbolPair.second;
        }
    }

    auto Emitter::EmitStructTypes(
        const std::vector<StructTypeSymbol*>& structSymbols
    ) -> void
    {
        std::for_each(begin(structSymbols), end(structSymbols),
        [&](const StructTypeSymbol* const structSymbol)
        {
            if (structSymbol->IsPrimitivelyEmittable())
                return;

            m_TypeMap[structSymbol] = llvm::StructType::create(
                *m_Compilation->LLVMContext,
                structSymbol->CreateSignature()
            );
        });

        std::for_each(begin(structSymbols), end(structSymbols),
        [&](const StructTypeSymbol* const structSymbol)
        {
            if (structSymbol->IsPrimitivelyEmittable())
                return;

            std::vector<llvm::Type*> elements{};

            const auto varSymbols = structSymbol->GetVars();
            std::for_each(begin(varSymbols), end(varSymbols),
            [&](const InstanceVarSymbol* const varSymbol)
            {
                auto* const type = GetIRType(varSymbol->GetType());
                elements.push_back(type);
            });

            auto* const structType = m_TypeMap.at(structSymbol);
            static_cast<llvm::StructType*>(structType)->setBody(elements);
        });
    }

    auto Emitter::EmitStaticVars(
        const std::vector<StaticVarSymbol*>& varSymbols
    ) -> void
    {
        std::for_each(begin(varSymbols), end(varSymbols),
        [&](const StaticVarSymbol* const varSymbol)
        {
            const auto name = varSymbol->CreateSignature();
            auto* const type = GetIRType(varSymbol->GetType());

            m_Module->getOrInsertGlobal(
                name,
                llvm::PointerType::get(type, 0)
            );

            auto* const var = m_Module->getNamedGlobal(name);
            var->setInitializer(llvm::Constant::getNullValue(var->getType()));
            var->setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);

            m_StaticVarMap[varSymbol] = var;
        });
    }

    auto Emitter::EmitFunctions(
        const std::vector<FunctionSymbol*>& functionSymbols
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
            begin(functionSymbols), 
            end  (functionSymbols), 
            back_inserter(functionsSymbolBlockPairs), 
            [&](FunctionSymbol* const functionSymbol)
            {
                std::vector<llvm::Type*> paramTypes{};
                const auto paramSymbols =
                    functionSymbol->CollectAllParams();
                std::transform(
                    begin(paramSymbols), 
                    end  (paramSymbols), 
                    back_inserter(paramTypes), 
                    [&](IParamVarSymbol* const paramSymbol)
                    {
                        return llvm::PointerType::get(
                            GetIRType(paramSymbol->GetType()), 
                            0
                        );
                    }
                );

                auto* const type = llvm::FunctionType::get(
                    GetIRType(functionSymbol->GetType()),
                    paramTypes,
                    false
                );

                auto* const function = llvm::Function::Create(
                    type,
                    llvm::Function::ExternalLinkage,
                    functionSymbol->CreateSignature(),
                    *m_Module
                );

                m_FunctionMap[functionSymbol] = function;

                auto* const block = llvm::BasicBlock::Create(
                    *m_Compilation->LLVMContext,
                    "",
                    function
                );

                return FunctionSymbolBlockPair
                { 
                    function,
                    functionSymbol,
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
            [&](FunctionSymbolBlockPair& functionSymbolBlockPair)
            {
                clearFunctionData();

                m_Function = functionSymbolBlockPair.Function;
                m_FunctionSymbol = functionSymbolBlockPair.Symbol;
                m_BlockBuilder = std::make_unique<BlockBuilder>(
                    functionSymbolBlockPair.Block
                );

                ACE_ASSERT(m_FunctionSymbol->GetBody().has_value());
                functionSymbolBlockPair.Symbol->GetBody().value()->Emit(*this);

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
