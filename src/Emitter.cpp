#include "Emitter.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <filesystem>
#include <fstream>
#include <climits>
#include <cinttypes>

#pragma warning(push, 0)

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

#pragma warning(pop)

#include "Scope.hpp"
#include "BoundNode/Module.hpp"
#include "BoundNode/Function.hpp"
#include "BoundNode/Variable/Normal/Static.hpp"
#include "BoundNode/Variable/Parameter/Normal.hpp"
#include "BoundNode/Statement/Block.hpp"
#include "BoundNode/Statement/Variable.hpp"
#include "BoundNode/Statement/Label.hpp"
#include "BoundNode/Statement/BlockEnd.hpp"
#include "BoundNode/Statement/Expression.hpp"
#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "BoundNode/Expression/VariableReference/Static.hpp"
#include "BoundNode/Expression/VariableReference/Instance.hpp"
#include "Log.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Variable/Normal/Static.hpp"
#include "Symbol/Variable/Normal/Instance.hpp"
#include "Symbol/Variable/Parameter/Base.hpp"
#include "Symbol/Variable/Local.hpp"
#include "Symbol/Type/Struct.hpp"
#include "Asserts.hpp"
#include "NativeSymbol.hpp"
#include "Utility.hpp"
#include "Name.hpp"
#include "SpecialIdentifier.hpp"
#include "Core.hpp"

namespace Ace
{
    Emitter::Emitter(
        const std::string& t_packageName
    ) : m_PackageName{ t_packageName },
        m_Context{ std::make_unique<llvm::LLVMContext>() },
        m_Module{ std::make_unique<llvm::Module>("module", *m_Context) },
        m_LabelBlockMap{ *this }
    {
        m_Module->setTargetTriple(llvm::sys::getProcessTriple());
    }

    Emitter::~Emitter()
    {
    }

    auto Emitter::Emit() -> Result
    {
        m_C.Initialize(*m_Context, *m_Module);

        const auto symbols = Scope::GetRoot()->CollectAllDefinedSymbolsRecursive();
        const auto typeSymbols = DynamicCastFilter<Symbol::Type::IBase*>(symbols);
        const auto structSymbols = DynamicCastFilter<Symbol::Type::Struct*>(typeSymbols);
        const auto variableSymbols = DynamicCastFilter<Symbol::Variable::Normal::Static*>(symbols);

        std::vector<const BoundNode::IBase*> nodes{}; 
        std::for_each(begin(m_ASTs), end(m_ASTs), [&]
        (const std::shared_ptr<const BoundNode::Module>& t_ast)
        {
            const auto children = t_ast->GetChildren();
            nodes.insert(end(nodes), begin(children), end(children));
        });
        const auto functionNodes = DynamicCastFilter<const BoundNode::Function*>(nodes);
       
        EmitNativeTypes();
        EmitStructTypes(structSymbols);

        EmitCopyGlue(structSymbols);

        EmitStaticVariables(variableSymbols);

        const auto functionSymbols = Scope::GetRoot()->CollectDefinedSymbolsRecursive<Symbol::Function>();
        EmitFunctions(functionSymbols);
        
        std::vector<const Symbol::Function*> mainFunctionSymbols{};
        std::for_each(begin(functionNodes), end(functionNodes), [&]
        (const BoundNode::Function* const t_functionNode)
        {
            if (t_functionNode->GetSymbol()->GetName() == SpecialIdentifier::Main) 
            {
                mainFunctionSymbols.push_back(t_functionNode->GetSymbol());
            }
        });

        ACE_ASSERT(mainFunctionSymbols.size() == 1);
        auto* const mainFunctionSymbol = mainFunctionSymbols.front();
        ACE_ASSERT(mainFunctionSymbol->GetType()->GetUnaliased() == NativeSymbol::Int.GetSymbol());
        ACE_ASSERT(!mainFunctionSymbol->IsInstance());
        ACE_ASSERT(mainFunctionSymbol->GetAllParameters().empty());

        auto* const mainType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*m_Context),
            false
        );

        auto* const mainFunction = llvm::Function::Create(
            mainType,
            llvm::Function::ExternalLinkage,
            "main",
            *m_Module
        );

        auto* const mainBlock = llvm::BasicBlock::Create(
            *m_Context,
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
        ACE_LOG_INFO(originalModuleString);
        originalModuleString.clear();

        const auto& now = std::chrono::steady_clock::now;

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
        
        auto mpm = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
        mpm.run(*m_Module, mam);

        const auto timeAnalysesEnd = now();

        std::string moduleString{};
        llvm::raw_string_ostream moduleOStream{ moduleString };
        moduleOStream << *m_Module;
        moduleOStream.flush();
        std::ofstream irFileStream
        { 
            "/home/samo/repos/ace/ace/build/" + m_PackageName + ".ll" 
        };
        ACE_ASSERT(irFileStream);
        irFileStream << moduleString;
        moduleString.clear();

        std::error_code errorCode{};
        llvm::raw_fd_ostream bitcodeFileOStream
        { 
            "/home/samo/repos/ace/ace/build/" + 
            m_PackageName + ".bc", errorCode, llvm::sys::fs::OF_None 
        };
        ACE_ASSERT(!errorCode);
        llvm::WriteBitcodeToFile(*m_Module, bitcodeFileOStream);
        bitcodeFileOStream.close();
        
        const auto timeLLCStart = now();

        const std::string llc = 
            "llc -O3 -opaque-pointers -relocation-model=pic -filetype=obj "
            "/home/samo/repos/ace/ace/build/" + m_PackageName + 
            ".bc -o /home/samo/repos/ace/ace/build/" + m_PackageName + ".obj";
        system(llc.c_str());

        const auto timeLLCEnd = now();

        const auto timeClangStart = now();

        const std::string clang = 
            "clang /home/samo/repos/ace/ace/build/" + m_PackageName + 
            ".obj -lc -lm -o /home/samo/repos/ace/ace/build/" + m_PackageName;
        system(clang.c_str());

        const auto timeClangEnd = now();

        return Result
        {
            Result::DurationInfo
            {
                timeAnalysesEnd - timeAnalysesStart,
                timeLLCEnd - timeLLCStart,
                timeClangEnd - timeClangStart
            }
        };
    }

    auto Emitter::EmitFunctionBodyStatements(
        const std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>& t_statements
    ) -> void
    {
        const auto parameterSymbols = m_FunctionSymbol->GetAllParameters();
        for (size_t i = 0; i < parameterSymbols.size(); i++)
        {
            auto* const parameterSymbol = parameterSymbols[i];
            auto* const typeSymbol = parameterSymbol->GetType();

            auto* const allocaInst = m_BlockBuilder->Builder.CreateAlloca(
                GetIRType(typeSymbol),
                nullptr,
                parameterSymbol->GetName()
            );

            EmitCopy(
                allocaInst,
                m_Function->arg_begin() + i,
                typeSymbol
            );

            m_LocalVariableMap[parameterSymbol] = allocaInst;
        }

        for (size_t i = 0; i < t_statements.size(); i++)
        {
            const auto* const statement = t_statements.at(i).get();

            ACE_ASSERT(!m_StatementIndexMap.contains(statement)); 
            m_StatementIndexMap[statement] = i;
        
            auto* const variableStatement = 
                dynamic_cast<const BoundNode::Statement::Variable*>(statement);

            if (variableStatement)
            {
                auto* const variableSymbol = variableStatement->GetSymbol();
                ACE_ASSERT(
                    !m_LocalVariableSymbolStatementIndexMap.contains(variableSymbol)
                );
                m_LocalVariableSymbolStatementIndexMap[variableSymbol] = i;
                // TODO: .emplace_back(...) doesn't work here. Fix.
                m_LocalVariableSymbolStatementIndexPairs.push_back(
                    LocalVariableSymbolStatementIndexPair{ variableSymbol, i }
                );
            }
        }

        std::for_each(
            begin(m_LocalVariableSymbolStatementIndexPairs), 
            end(m_LocalVariableSymbolStatementIndexPairs), 
            [&](const LocalVariableSymbolStatementIndexPair& t_symbolIndexPair)
            {
                auto* const variableSymbol = t_symbolIndexPair.LocalVariableSymbol;
                auto* const type = GetIRType(variableSymbol->GetType());
                ACE_ASSERT(!m_LocalVariableMap.contains(variableSymbol));
                m_LocalVariableMap[variableSymbol] = m_BlockBuilder->Builder.CreateAlloca(
                    type,
                    nullptr,
                    variableSymbol->GetName()
                );
            }
        );

        const auto blockStartIndicies = [&]() -> std::vector<size_t>
        {
            std::vector<size_t> indicies{};

            if (!t_statements.empty())
            {
                indicies.push_back(0);
            }

            for (size_t i = 0; i < t_statements.size(); i++)
            {
                auto* const labelStatement =
                    dynamic_cast<const BoundNode::Statement::Label*>(t_statements.at(i).get());

                if (labelStatement)
                {
                    indicies.push_back(i);
                }
            }

            return indicies;
        }();

        for (size_t i = 0; i < blockStartIndicies.size(); i++)
        {
            const bool isLastBlock = i == (blockStartIndicies.size() - 1);

            const auto beginStatementIt = begin(t_statements) + blockStartIndicies.at(i);
            const auto endStatementIt   = isLastBlock ?
                                          end  (t_statements) :
                                          begin(t_statements) + blockStartIndicies.at(i + 1);

            std::for_each(beginStatementIt, endStatementIt, [&]
            (const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
            {
                if (t_statement.get() == beginStatementIt->get())
                {
                    auto* const labelStatement =
                        dynamic_cast<const BoundNode::Statement::Label*>(t_statement.get());

                    if (labelStatement)
                    {
                        auto blockBuilder = std::make_unique<BlockBuilder>(
                            GetLabelBlockMap().GetOrCreateAt(labelStatement->GetLabelSymbol())
                        );

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

                t_statement->Emit(*this);
                
                auto* const blockEndStatement = 
                    dynamic_cast<const BoundNode::Statement::BlockEnd*>(t_statement.get());

                if (blockEndStatement)
                {
                    auto blockVariableSymbols = 
                        blockEndStatement->GetSelfScope()->CollectDefinedSymbols<Symbol::Variable::Local>();

                    std::sort(begin(blockVariableSymbols), end(blockVariableSymbols), [&]
                    (const Symbol::Variable::Local* const t_lhs, const Symbol::Variable::Local* const t_rhs)
                    {
                        return
                            m_LocalVariableSymbolStatementIndexMap.at(t_lhs) >
                            m_LocalVariableSymbolStatementIndexMap.at(t_rhs);
                    });

                    std::for_each(begin(blockVariableSymbols), end(blockVariableSymbols), [&]
                    (Symbol::Variable::Local* const t_variableSymbol)
                    {
                        EmitDrop({ 
                            m_LocalVariableMap.at(t_variableSymbol), 
                            t_variableSymbol->GetType() 
                        });
                    });
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
                EmitDropArguments();

                m_BlockBuilder->Builder.CreateRetVoid();
            }
        }
    }

    auto Emitter::EmitLoadArgument(
        const size_t& t_index, 
        llvm::Type* const t_type
    ) const -> llvm::Value*
    {
        return m_BlockBuilder->Builder.CreateLoad(
            t_type,
            m_Function->arg_begin() + t_index
        );
    }

    auto Emitter::EmitDrop(const ExpressionDropData& t_dropData) -> void
    {
        if (t_dropData.TypeSymbol->IsReference())
            return;

        auto operatorName = t_dropData.TypeSymbol->GetFullyQualifiedName();
        operatorName.Sections.push_back(Name::Symbol::Section{ SpecialIdentifier::Operator::Drop });

        const auto expOperatorSymbol = 
            t_dropData.TypeSymbol->GetScope()->ResolveStaticSymbol<Symbol::Function>(operatorName);

        // TODO: Force every type to have a drop (for members).
        if (!expOperatorSymbol)
            return;

        auto* const typeSymbol = expOperatorSymbol.Unwrap()->GetParameters().front()->GetType();
        auto* const type = GetIRType(typeSymbol);

        auto* const allocaInst = m_BlockBuilder->Builder.CreateAlloca(type);
        m_BlockBuilder->Builder.CreateStore(t_dropData.Value, allocaInst);

        m_BlockBuilder->Builder.CreateCall(
            m_FunctionMap.at(expOperatorSymbol.Unwrap()),
            { allocaInst }
        );
    }

    auto Emitter::EmitDropTemporaries(const std::vector<ExpressionDropData>& t_temporaries) -> void
    {
        std::for_each(rbegin(t_temporaries), rend(t_temporaries), [&]
        (const ExpressionDropData& t_temporary)
        {
            EmitDrop(t_temporary);
        });
    }

    auto Emitter::EmitDropLocalVariablesBeforeStatement(
        const BoundNode::Statement::IBase* const t_statement
    ) -> void
    {
        const auto statementIndex = m_StatementIndexMap.at(t_statement);
        auto* scope = t_statement->GetScope();

        std::for_each(
            rbegin(m_LocalVariableSymbolStatementIndexPairs), 
            rend(m_LocalVariableSymbolStatementIndexPairs), [&]
            (const LocalVariableSymbolStatementIndexPair& t_symbolIndexPair)
            {
                const auto variableStatementIndex = t_symbolIndexPair.StatementIndex;

                if (variableStatementIndex >= statementIndex)
                    return;

                auto* const variableSymbol = t_symbolIndexPair.LocalVariableSymbol;

                if (variableSymbol->GetScope() != scope)
                {
                    if (variableSymbol->GetScope()->GetNestLevel() >= scope->GetNestLevel())
                        return;

                    scope = variableSymbol->GetScope();
                }

                EmitDrop({
                    m_LocalVariableMap.at(variableSymbol),
                    variableSymbol->GetType()
                });
            }
        );

        EmitDropArguments();
    }

    auto Emitter::EmitDropArguments() -> void
    {
        const auto parameterSymbols = m_FunctionSymbol->GetAllParameters();
        std::for_each(rbegin(parameterSymbols), rend(parameterSymbols), [&]
        (Symbol::Variable::Parameter::IBase* const t_parameterSymbol)
        {
            EmitDrop({
                m_LocalVariableMap.at(t_parameterSymbol),
                t_parameterSymbol->GetType()
            });
        });
    }

    auto Emitter::EmitCopy(
        llvm::Value* const t_lhsValue, 
        llvm::Value* const t_rhsValue, 
        Symbol::Type::IBase* const t_typeSymbol
    ) -> void
    {
        auto [scope, partialSignature] = [&]() -> std::pair<Scope*, std::string>
        {
            if (t_typeSymbol->IsReference())
            {
                auto* const pointerTypeSymbol = NativeSymbol::Pointer.GetSymbol();
                return { pointerTypeSymbol->GetScope(), pointerTypeSymbol->CreatePartialSignature() };
            }

            return { t_typeSymbol->GetUnaliased()->GetScope(), t_typeSymbol->CreatePartialSignature() };
        }();

        auto* const glueSymbol = scope->ExclusiveResolveSymbol<Symbol::Function>(
            SpecialIdentifier::CreateCopyGlue(partialSignature)
        ).Unwrap();

        auto* const type = GetIRType(t_typeSymbol);
        auto* const pointerType = llvm::PointerType::get(type, 0);

        auto* const lhsAllocaInst = m_BlockBuilder->Builder.CreateAlloca(pointerType);
        m_BlockBuilder->Builder.CreateStore(t_lhsValue, lhsAllocaInst);

        auto* const rhsAllocaInst = m_BlockBuilder->Builder.CreateAlloca(pointerType);
        m_BlockBuilder->Builder.CreateStore(t_rhsValue, rhsAllocaInst);

        m_BlockBuilder->Builder.CreateCall(
            m_FunctionMap.at(glueSymbol),
            { lhsAllocaInst, rhsAllocaInst }
        );
    }

    auto Emitter::GetIRType(const Symbol::Type::IBase* const t_typeSymbol) const -> llvm::Type*
    {
        if (t_typeSymbol->IsReference())
        {
            return llvm::PointerType::get(
                m_TypeMap.at(t_typeSymbol->GetWithoutReference()->GetUnaliased()),
                0
            );
        }
        else
        {
            return m_TypeMap.at(t_typeSymbol->GetUnaliased());
        }
    }

    auto Emitter::EmitCopyGlue(const std::vector<Symbol::Type::Struct*>& t_structSymbols) -> void
    {
        struct GlueStructSymbolPair
        {
            Symbol::Function* GlueSymbol{};
            Symbol::Type::Struct* StructSymbol{};
        };

        std::vector<GlueStructSymbolPair> glueStructSymbolPairs{};

        std::for_each(begin(t_structSymbols), end(t_structSymbols), [&]
        (Symbol::Type::Struct* const t_structSymbol)
        {
            if (t_structSymbol == NativeSymbol::Void.GetSymbol())
                return;

            if (t_structSymbol->IsReference())
                return;

            glueStructSymbolPairs.push_back(GlueStructSymbolPair{
                DefineCopyGlueSymbols(t_structSymbol),
                t_structSymbol
	    });
        });

        std::for_each(begin(glueStructSymbolPairs), end(glueStructSymbolPairs), [&]
        (const GlueStructSymbolPair& t_glueStructSymbolPair)
        {
            auto* const glueSymbol = t_glueStructSymbolPair.GlueSymbol;
            auto* const structSymbol = t_glueStructSymbolPair.StructSymbol;

            const auto body = structSymbol->IsTriviallyCopyable() ? 
                CreateTrivialCopyGlueBody(glueSymbol, structSymbol) :
                CreateCopyGlueBody(glueSymbol, structSymbol);

            glueSymbol->SetBody(body);
        });
    }

    auto Emitter::EmitNativeTypes() -> void
    {
        for (auto& typeSymbolPair : NativeSymbol::GetIRTypeSymbolMap(*this))
        {
            m_TypeMap[typeSymbolPair.first] = typeSymbolPair.second;
        }
    }

    auto Emitter::EmitStructTypes(const std::vector<Symbol::Type::Struct*>& t_structSymbols) -> void
    {
        std::for_each(begin(t_structSymbols), end(t_structSymbols), [&]
        (const Symbol::Type::Struct* const t_structSymbol)
        {
            if (t_structSymbol->IsNativeSized())
                return;

            m_TypeMap[t_structSymbol] = llvm::StructType::create(
                *m_Context,
                t_structSymbol->CreateSignature()
            );
        });

        std::for_each(begin(t_structSymbols), end(t_structSymbols), [&]
        (const Symbol::Type::Struct* const t_structSymbol)
        {
            if (t_structSymbol->IsNativeSized())
                return;

            std::vector<llvm::Type*> elements{};

            const auto variableSymbols = t_structSymbol->GetVariables();
            std::for_each(begin(variableSymbols), end(variableSymbols), [&]
            (const Symbol::Variable::Normal::Instance* const t_variableSymbol)
            {
                auto* const type = GetIRType(t_variableSymbol->GetType());
                elements.push_back(type);
            });

            static_cast<llvm::StructType*>(m_TypeMap.at(t_structSymbol))->setBody(elements);
        });
    }

    auto Emitter::EmitStaticVariables(
        const std::vector<Symbol::Variable::Normal::Static*>& t_variableSymbols
    ) -> void
    {
        std::for_each(begin(t_variableSymbols), end(t_variableSymbols), [&]
        (const Symbol::Variable::Normal::Static* const t_variableSymbol)
        {
            auto* const type = GetIRType(t_variableSymbol->GetType());

            auto* const variable = m_Module->getOrInsertGlobal(
                t_variableSymbol->CreateSignature(),
                llvm::PointerType::get(type, 0)
            );

            m_StaticVariableMap[t_variableSymbol] = variable;
        });
    }

    auto Emitter::EmitFunctions(const std::vector<Symbol::Function*>& t_functionSymbols) -> void
    {
        struct FunctionSymbolBlockPair
        {
            llvm::Function* Function{};
            Symbol::Function* Symbol{};
            llvm::BasicBlock* Block{};
        };

        std::vector<FunctionSymbolBlockPair> functionsSymbolBlockPairs{};
        std::transform(
            begin(t_functionSymbols), 
            end(t_functionSymbols), 
            back_inserter(functionsSymbolBlockPairs), 
            [&](Symbol::Function* const t_functionSymbol)
            {
                std::vector<llvm::Type*> parameterTypes{};
                const auto parameterSymbols = t_functionSymbol->GetAllParameters();
                std::transform(
                    begin(parameterSymbols), 
                    end(parameterSymbols), 
                    back_inserter(parameterTypes), 
                    [&](Symbol::Variable::Parameter::IBase* const t_parameterSymbol)
                    {
                        return llvm::PointerType::get(GetIRType(t_parameterSymbol->GetType()), 0);
                    }
                );

                auto* const type = llvm::FunctionType::get(
                    GetIRType(t_functionSymbol->GetType()),
                    parameterTypes,
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
                    *m_Context,
                    "",
                    function
                );

                return FunctionSymbolBlockPair{ function, t_functionSymbol, block };
            }
        );

        auto clearFunctionData = [&]() -> void
        {
            m_Function = nullptr;
            m_FunctionSymbol = nullptr;
            m_BlockBuilder = nullptr;

            m_LocalVariableMap.clear();
            m_LabelBlockMap.Clear();
            m_StatementIndexMap.clear();
            m_LocalVariableSymbolStatementIndexMap.clear();
            m_LocalVariableSymbolStatementIndexPairs.clear();
        };

        std::for_each(begin(functionsSymbolBlockPairs), end(functionsSymbolBlockPairs), [&]
        (FunctionSymbolBlockPair& t_functionSymbolBlockPair)
        {
            clearFunctionData();

            m_Function = t_functionSymbolBlockPair.Function;
            m_FunctionSymbol = t_functionSymbolBlockPair.Symbol;
            m_BlockBuilder = std::make_unique<BlockBuilder>(t_functionSymbolBlockPair.Block);

            ACE_LOG_INFO("Emitting function: " << m_FunctionSymbol->CreateSignature());

            ACE_ASSERT(m_FunctionSymbol->GetBody().has_value());
            t_functionSymbolBlockPair.Symbol->GetBody().value()->Emit(*this);

            if (!m_FunctionSymbol->IsNative() && !m_BlockBuilder->Block->back().isTerminator())
            {
                m_BlockBuilder->Builder.CreateUnreachable();
            }
        });

        clearFunctionData();
    }

    auto Emitter::DefineCopyGlueSymbols(
        Symbol::Type::Struct* const t_structSymbol
    ) const -> Symbol::Function*
    {
        auto* const selfScope = t_structSymbol->GetScope()->GetOrCreateChild({});

        auto ownedGlueSymbol = std::make_unique<Symbol::Function>(
            selfScope,
            SpecialIdentifier::CreateCopyGlue(t_structSymbol->CreatePartialSignature()),
            AccessModifier::Public,
            false,
            NativeSymbol::Void.GetSymbol()
        );

        auto* const glueSymbol = dynamic_cast<Symbol::Function*>(
            Scope::DefineSymbol(std::move(ownedGlueSymbol)).Unwrap()
        );

        Scope::DefineSymbol(std::make_unique<Symbol::Variable::Parameter::Normal>(
            selfScope,
            SpecialIdentifier::CreateAnonymous(),
            t_structSymbol->GetWithReference(),
            0
        )).Unwrap();
        Scope::DefineSymbol(std::make_unique<Symbol::Variable::Parameter::Normal>(
            selfScope,
            SpecialIdentifier::CreateAnonymous(),
            t_structSymbol->GetWithReference(),
            1
        )).Unwrap();

        return glueSymbol;
    }

    auto Emitter::CreateTrivialCopyGlueBody(
        Symbol::Function* const t_glueSymbol,
        Symbol::Type::Struct* const t_structSymbol
    ) const -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialCopyGlueBodyEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialCopyGlueBodyEmitter(Symbol::Type::Struct* const t_structSymbol)
                : m_StructSymbol{ t_structSymbol }
            {
            }

            auto Emit(Emitter& t_emitter) const -> void final
            {
                auto* const structType = t_emitter.GetIRType(m_StructSymbol);
                auto* const structPtrType = llvm::PointerType::get(
                    structType,
                    0
                );
                
                auto* const selfPtr = t_emitter.EmitLoadArgument(
                    0,
                    structPtrType
                );

                auto* const otherPtr = t_emitter.EmitLoadArgument(
                    1, 
                    structPtrType
                );
                auto* const otherValue = t_emitter.GetBlockBuilder().Builder.CreateLoad(
                    structType,
                    otherPtr
                );

                t_emitter.GetBlockBuilder().Builder.CreateStore(
                    otherValue,
                    selfPtr
                );

                t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }

        private:
            Symbol::Type::Struct* m_StructSymbol{};
        };

        return std::make_shared<const TrivialCopyGlueBodyEmitter>(t_structSymbol);
    }
    auto Emitter::CreateCopyGlueBody(
        Symbol::Function* const t_glueSymbol,
        Symbol::Type::Struct* const t_structSymbol
    ) const -> std::shared_ptr<const IEmittable<void>>
    {
        auto* const bodyScope = t_glueSymbol->GetSelfScope()->GetOrCreateChild({});

        const auto parameterSymbols = t_glueSymbol->GetParameters();
        const auto selfParameterReferenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Static>(
            bodyScope,
            parameterSymbols.at(0)
        );
        const auto otherParameterReferenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Static>(
            bodyScope,
            parameterSymbols.at(1)
        );

        auto operatorName = t_structSymbol->GetFullyQualifiedName();
        operatorName.Sections.emplace_back(SpecialIdentifier::Operator::Copy);

        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> statements{};
        if (const auto expOperatorSymbol = Scope::GetRoot()->ResolveStaticSymbol<Symbol::Function>(operatorName))
        {
            std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> arguments{};
            arguments.push_back(selfParameterReferenceExpressionNode);
            arguments.push_back(otherParameterReferenceExpressionNode);

            const auto functionCallExpressionNode = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
                bodyScope,
                expOperatorSymbol.Unwrap(),
                arguments
            );

            const auto expressionStatementNode = std::make_shared<const BoundNode::Statement::Expression>(
                functionCallExpressionNode
            );

            statements.push_back(expressionStatementNode);
        }
        else
        {
            const auto variableSymbols = t_structSymbol->GetVariables();
            std::for_each(rbegin(variableSymbols), rend(variableSymbols), [&]
            (Symbol::Variable::Normal::Instance* const t_variableSymbol)
            {
                auto* const variableTypeSymbol = t_variableSymbol->GetType();
                auto* const variableTypeScope = variableTypeSymbol->GetUnaliased()->GetScope();
                auto* const variableGlueSymbol = variableTypeScope->ExclusiveResolveSymbol<Symbol::Function>(
                    SpecialIdentifier::CreateCopyGlue(variableTypeSymbol->CreatePartialSignature())
                ).Unwrap();
                
                const auto selfParameterVariableRerefenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
                    selfParameterReferenceExpressionNode,
                    t_variableSymbol
                );
                const auto otherParameterVariableRerefenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
                    otherParameterReferenceExpressionNode,
                    t_variableSymbol
                );

                std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> arguments{};
                arguments.push_back(selfParameterVariableRerefenceExpressionNode);
                arguments.push_back(otherParameterVariableRerefenceExpressionNode);

                const auto functionCallExpressionNode = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
                    bodyScope,
                    variableGlueSymbol,
                    arguments
                );

                const auto expressionStatementNode = std::make_shared<const BoundNode::Statement::Expression>(
                    functionCallExpressionNode
                );

                statements.push_back(expressionStatementNode);
            });
        }

        const auto bodyNode = std::make_shared<const BoundNode::Statement::Block>(
            bodyScope->GetParent(),
            statements
        );

        return Core::CreateTransformedAndVerifiedAST(
            bodyNode,
            [&](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            { 
                return t_bodyNode->GetOrCreateTypeChecked({ NativeSymbol::Void.GetSymbol() }); 
            },
            [](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateLowered({});
            },
            [&](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateTypeChecked({ NativeSymbol::Void.GetSymbol() });
            }
        ).Unwrap();
    }
}
