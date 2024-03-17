#include "Emitter.hpp"

#include <memory>
#include <vector>
#include <map>
#include <string_view>
#include <chrono>
#include <fstream>

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
#include "Semas/All.hpp"
#include "Log.hpp"
#include "Assert.hpp"
#include "DynamicCastFilter.hpp"
#include "Name.hpp"
#include "AnonymousIdent.hpp"
#include "SpecialIdent.hpp"
#include "Compilation.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/EmittingDiagnostics.hpp"

namespace Ace
{
    auto EmittingBlock::IsTerminated() const -> bool
    {
        return !Block->empty() && Block->back().isTerminator();
    }

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
            m_Emitter.GetContext(),
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
        Compilation* const compilation
    ) : m_Compilation{ compilation },
        m_Module
        {
            std::make_unique<llvm::Module>(
                "module",
                m_Context
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

    static auto SaveModuleToFile(
        Compilation* const compilation,
        const llvm::Module& module,
        const std::filesystem::path& filePath
    ) -> void
    {
        std::string moduleString{};

        llvm::raw_string_ostream moduleOStream{ moduleString };
        moduleOStream << module;
        moduleOStream.flush();

        std::ofstream irFileStream{ filePath };
        ACE_ASSERT(irFileStream);
        irFileStream << moduleString;

        moduleString.clear();
    }

    static auto IsConcreteSymbol(ISymbol* const symbol) -> bool
    {
        if (symbol->IsError())
        {
            return false;
        }

        auto* const genericSymbol = dynamic_cast<IGenericSymbol*>(symbol);
        if (!genericSymbol)
        {
            return true;
        }

        return !genericSymbol->IsPlaceholder();
    }

    auto Emitter::Emit() -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto& globalScope = GetCompilation()->GetGlobalScope();
        const auto& packageName = GetCompilation()->GetPackage().Name;

        m_C.Initialize(GetContext(), GetModule());

        const auto allSymbols = globalScope->CollectAllSymbolsRecursive();
        std::set<ISymbol*> symbolSet{};
        std::for_each(begin(allSymbols), end(allSymbols),
        [&](ISymbol* const symbol)
        {
            if (IsConcreteSymbol(symbol))
            {
                symbolSet.insert(symbol->GetUnaliased());
            }
        });
        const std::vector<ISymbol*> symbols{ begin(symbolSet), end(symbolSet) };

        auto* const mainType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(GetContext()),
            false
        );
        auto* const mainFunction = llvm::Function::Create(
            mainType,
            llvm::Function::ExternalLinkage,
            "main",
            GetModule()
        );
        SetBlock(std::make_unique<EmittingBlock>(
            llvm::BasicBlock::Create(GetContext(), "", mainFunction)
        ));
       
        const auto typeSymbols = DynamicCastFilter<ITypeSymbol*>(symbols);

        EmitNativeTypes();
        EmitStructTypes(DynamicCastFilter<StructTypeSymbol*>(typeSymbols));

        EmitGlobalVars(DynamicCastFilter<GlobalVarSymbol*>(symbols));

        const auto allFunctionSymbols =
            globalScope->CollectSymbolsRecursive<FunctionSymbol>();

        std::vector<FunctionSymbol*> functionSymbols{};
        std::copy_if(
            begin(allFunctionSymbols),
            end  (allFunctionSymbols),
            back_inserter(functionSymbols),
            IsConcreteSymbol
        );

        std::vector<FunctionHeader> functionHeaders{};
        std::transform(
            begin(functionSymbols),
            end  (functionSymbols),
            back_inserter(functionHeaders),
            [&](FunctionSymbol* const symbol)
            {
                return EmitFunctionHeader(symbol);
            }
        );

        EmitVtbls(DynamicCastFilter<TraitImplSymbol*>(symbols));
        EmitTypeInfos(typeSymbols);

        const auto mainFunctionSymbolIt = std::find_if(
            begin(functionSymbols),
            end  (functionSymbols),
            [&](FunctionSymbol* const functionSymbol)
            {
                const auto packageScope =
                    globalScope->GetOrCreateChild(packageName);

                return
                    (functionSymbol->GetScope() == packageScope) &&
                    (functionSymbol->GetName().String == SpecialIdent::Main);
            }
        );
        ACE_ASSERT(mainFunctionSymbolIt != end(functionSymbols));
        auto* const mainFunctionSymbol = *mainFunctionSymbolIt;
        ACE_ASSERT(
            mainFunctionSymbol->GetType()->GetUnaliased() == 
            GetCompilation()->GetNatives().Int.GetSymbol()
        );
        ACE_ASSERT(mainFunctionSymbol->GetCategory() == SymbolCategory::Static);
        ACE_ASSERT(mainFunctionSymbol->CollectAllParams().empty());
        GetBlock().Builder.CreateRet(GetBlock().Builder.CreateCall(
            GetFunction(mainFunctionSymbol)
        ));

        std::for_each(begin(functionHeaders), end(functionHeaders),
        [&](const FunctionHeader& header)
        {
            EmitFunctionBlock(header);
        });

        std::for_each(begin(functionSymbols), end(functionSymbols),
        [&](FunctionSymbol* const functionSymbol)
        {
            if (functionSymbol != functionSymbol->GetRoot())
            {
                return;
            }

            if (!functionSymbol->GetEmittableBlock().has_value())
            {
                diagnostics.Add(CreateMissingFunctionBlockError(
                    functionSymbol
                ));
            }
        });

        const auto bcFilePath    = CreateOutputFilePath(packageName, "bc");
        const auto llFilePath    = CreateOutputFilePath(packageName, "ll");
        const auto optLlFilePath = CreateOutputFilePath(packageName, "opt.ll");
        const auto objFilePath   = CreateOutputFilePath(packageName, "obj");
        const auto exeFilePath   = CreateOutputFilePath(packageName, "");

        std::string originalModuleString{};
        llvm::raw_string_ostream originalModuleOStream{ originalModuleString };
        originalModuleOStream << GetModule();
        originalModuleOStream.flush();
        originalModuleString.clear();

        // TODO: Make this optional with a CLI option
        SaveModuleToFile(GetCompilation(), GetModule(), llFilePath);

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
        
        auto mpm =
            pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);

        mpm.run(GetModule(), mam);

        // TODO: Make this optional with a CLI option
        SaveModuleToFile(GetCompilation(), GetModule(), optLlFilePath);

        std::error_code errorCode{};
        llvm::raw_fd_ostream bitcodeFileOStream
        { 
            bcFilePath.string(),
            errorCode, 
            llvm::sys::fs::OF_None,
        };
        ACE_ASSERT(!errorCode);
        llvm::WriteBitcodeToFile(GetModule(), bitcodeFileOStream);
        bitcodeFileOStream.close();

        system((
            "llc -O3 -opaque-pointers -relocation-model=pic -filetype=obj -o " +
            objFilePath.string() + " " + bcFilePath.string()
        ).c_str());

        system((
            "clang -lc -lm -o " + exeFilePath.string() + " " +
            objFilePath.string()
        ).c_str());

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return Void{ std::move(diagnostics) };
    }

    auto Emitter::EmitFunctionBlockStmts(
        const std::vector<std::shared_ptr<const IStmtSema>>& stmts
    ) -> void
    {
        auto* const rootFunctionSymbol =
            dynamic_cast<FunctionSymbol*>(m_FunctionSymbol->GetRoot());
        ACE_ASSERT(rootFunctionSymbol);

        const auto paramSymbols = rootFunctionSymbol->CollectAllParams();
        for (size_t i = 0; i < paramSymbols.size(); i++)
        {
            auto* const paramSymbol = paramSymbols.at(i);
            auto* const typeSymbol = CreateInstantiated<ISizedTypeSymbol>(
                paramSymbol->GetSizedType()
            );

            auto* const allocaInst = GetBlock().Builder.CreateAlloca(
                GetType(typeSymbol),
                nullptr,
                paramSymbol->GetName().String
            );

            EmitCopy(allocaInst, m_Function->arg_begin() + i, typeSymbol);

            m_LocalVarMap[paramSymbol] = allocaInst;
        }

        for (size_t i = 0; i < stmts.size(); i++)
        {
            const auto* const stmt = stmts.at(i).get();

            ACE_ASSERT(!m_StmtIndexMap.contains(stmt)); 
            m_StmtIndexMap[stmt] = i;
        
            if (auto* const varStmt = dynamic_cast<const VarStmtSema*>(stmt))
            {
                auto* const varSymbol = varStmt->GetSymbol();
                ACE_ASSERT(!m_LocalVarSymbolStmtIndexMap.contains(varSymbol));
                m_LocalVarSymbolStmtIndexMap[varSymbol] = i;
                m_LocalVarSymbolStmtIndexPairs.emplace_back(varSymbol, i);
            }
        }

        std::for_each(
            begin(m_LocalVarSymbolStmtIndexPairs), 
            end  (m_LocalVarSymbolStmtIndexPairs), 
            [&](const LocalVarSymbolStmtIndexPair& symbolIndexPair)
            {
                auto* const varSymbol = symbolIndexPair.LocalVarSymbol;
                auto* const type = GetType(varSymbol->GetType());
                ACE_ASSERT(!m_LocalVarMap.contains(varSymbol));
                m_LocalVarMap[varSymbol] = GetBlock().Builder.CreateAlloca(
                    type,
                    nullptr,
                    varSymbol->GetName().String
                );
            }
        );

        const auto blockBeginIndices = [&]() -> std::vector<size_t>
        {
            std::vector<size_t> indices{};

            if (!stmts.empty())
            {
                indices.push_back(0);
            }

            for (size_t i = 0; i < stmts.size(); i++)
            {
                auto* const stmt = stmts.at(i).get();
                auto* const labelStmt =
                    dynamic_cast<const LabelStmtSema*>(stmt);

                if (labelStmt)
                {
                    indices.push_back(i);
                }
            }

            return indices;
        }();

        for (size_t i = 0; i < blockBeginIndices.size(); i++)
        {
            const bool isLastBlock = i == (blockBeginIndices.size() - 1);

            const auto beginStmtIt = begin(stmts) + blockBeginIndices.at(i);
            const auto endStmtIt   = isLastBlock ?
                                          end  (stmts) :
                                          begin(stmts) + blockBeginIndices.at(i + 1);

            std::for_each(beginStmtIt, endStmtIt,
            [&](const std::shared_ptr<const IStmtSema>& stmt)
            {
                if (stmt.get() == beginStmtIt->get())
                {
                    auto* const labelStmt =
                        dynamic_cast<const LabelStmtSema*>(stmt.get());

                    if (labelStmt)
                    {
                        auto* const labelSymbol = labelStmt->GetSymbol();
                        auto block = std::make_unique<EmittingBlock>(
                            GetLabelBlockMap().GetOrCreateAt(labelSymbol)
                        );

                        if (!GetBlock().IsTerminated())
                        {
                            GetBlock().Builder.CreateBr(block->Block);
                        }

                        SetBlock(std::move(block));
                    }
                }

                if (GetBlock().IsTerminated())
                {
                    return;
                }

                stmt->Emit(*this);
                
                auto* const blockEndStmt = 
                    dynamic_cast<const BlockEndStmtSema*>(stmt.get());

                if (blockEndStmt)
                {
                    const auto blockScope = blockEndStmt->GetBodyScope();
                    auto blockVarSymbols = 
                        blockScope->CollectSymbols<LocalVarSymbol>();

                    std::sort(begin(blockVarSymbols), end(blockVarSymbols),
                    [&](
                        const LocalVarSymbol* const lhs,
                        const LocalVarSymbol* const rhs
                        )
                    {
                        return
                            m_LocalVarSymbolStmtIndexMap.at(lhs) >
                            m_LocalVarSymbolStmtIndexMap.at(rhs);
                    });

                    std::for_each(begin(blockVarSymbols), end(blockVarSymbols),
                    [&](LocalVarSymbol* const varSymbol)
                    {
                        EmitDrop({ 
                            m_LocalVarMap.at(varSymbol), 
                            varSymbol->GetType() 
                        });
                    });
                }
            });
        }

        if (
            m_Function->getReturnType()->isVoidTy() &&
            !GetBlock().IsTerminated()
            )
        {
            EmitDropArgs();
            GetBlock().Builder.CreateRetVoid();
        }
    }

    auto Emitter::EmitCall(
        ICallableSymbol* callableSymbol,
        const std::vector<llvm::Value*>& args
    ) -> llvm::Value*
    {
        callableSymbol = CreateInstantiated<ICallableSymbol>(callableSymbol);

        auto* const functionSymbol =
            dynamic_cast<FunctionSymbol*>(callableSymbol);
        if (functionSymbol)
        {
            return EmitStaticCall(functionSymbol, args);
        }

        auto* const prototypeSymbol =
            dynamic_cast<PrototypeSymbol*>(callableSymbol);
        ACE_ASSERT(prototypeSymbol);

        const auto isDynType = dynamic_cast<TraitTypeSymbol*>(
            prototypeSymbol->GetSelfType()->GetDerefed()
        ) != nullptr;

        if (!isDynType)
        {
            auto* const functionSymbol = Scope::CollectImplOfFor(
                prototypeSymbol,
                prototypeSymbol->GetSelfType()
            ).value();

            return EmitStaticCall(functionSymbol, args);
        }

        return EmitDynCall(prototypeSymbol, args);
    }

    auto Emitter::EmitLoadArg(
        const size_t index, 
        llvm::Type* const type
    ) -> llvm::Value*
    {
        return GetBlock().Builder.CreateLoad(
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
        auto* const concreteTypeSymbol = CreateInstantiated<IConcreteTypeSymbol>(
            typeSymbol->GetUnaliasedType()
        );
        ACE_ASSERT(concreteTypeSymbol);

        auto* const type = GetType(concreteTypeSymbol);

        if (typeSymbol->IsRef())
        {
            auto* const loadInst = GetBlock().Builder.CreateLoad(
                type,
                rhsValue
            );

            GetBlock().Builder.CreateStore(loadInst, lhsValue);
            return;
        }


        auto* const glueSymbol = concreteTypeSymbol->GetCopyGlue().value();

        auto* const lhsAllocaInst = GetBlock().Builder.CreateAlloca(
            GetPtrType()
        );
        GetBlock().Builder.CreateStore(lhsValue, lhsAllocaInst);

        auto* const rhsAllocaInst = GetBlock().Builder.CreateAlloca(
            GetPtrType()
        );
        GetBlock().Builder.CreateStore(rhsValue, rhsAllocaInst);

        GetBlock().Builder.CreateCall(
            m_FunctionMap.at(glueSymbol),
            { lhsAllocaInst, rhsAllocaInst }
        );
    }

    auto Emitter::EmitDrop(const ExprDropInfo& info) -> void
    {
        if (info.TypeSymbol->IsRef())
        {
            return;
        }

        auto* const typeSymbol = CreateInstantiated<IConcreteTypeSymbol>(
            info.TypeSymbol->GetUnaliasedType()
        );

        const auto glueSymbol = typeSymbol->GetDropGlue().value();

        auto* const refTypeSymbol =
            glueSymbol->CollectParams().front()->GetType();
        auto* const refType = GetType(refTypeSymbol);

        auto* const allocaInst = GetBlock().Builder.CreateAlloca(refType);
        GetBlock().Builder.CreateStore(info.Value, allocaInst);

        GetBlock().Builder.CreateCall(
            m_FunctionMap.at(glueSymbol),
            { allocaInst }
        );
    }

    auto Emitter::EmitDropTmps(const std::vector<ExprDropInfo>& tmps) -> void
    {
        std::for_each(rbegin(tmps), rend(tmps),
        [&](const ExprDropInfo& tmp)
        {
            EmitDrop(tmp);
        });
    }

    auto Emitter::EmitDropLocalVarsBeforeStmt(
        const IStmtSema* const stmt
    ) -> void
    {
        const auto stmtIndex = m_StmtIndexMap.at(stmt);
        auto scope = stmt->GetScope();

        std::for_each(
            rbegin(m_LocalVarSymbolStmtIndexPairs), 
            rend  (m_LocalVarSymbolStmtIndexPairs),
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

                EmitDrop({ m_LocalVarMap.at(varSymbol), varSymbol->GetType() });
            }
        );

        EmitDropArgs();
    }

    auto Emitter::EmitDropArgs() -> void
    {
        auto* const rootFunctionSymbol =
            dynamic_cast<FunctionSymbol*>(m_FunctionSymbol->GetRoot());
        ACE_ASSERT(rootFunctionSymbol);

        const auto paramSymbols = rootFunctionSymbol->CollectAllParams();
        std::for_each(rbegin(paramSymbols), rend(paramSymbols),
        [&](IParamVarSymbol* const paramSymbol)
        {
            auto* const typeSymbol =
                CreateInstantiated<ITypeSymbol>(paramSymbol->GetType());

            EmitDrop({ m_LocalVarMap.at(paramSymbol), typeSymbol });
        });
    }

    auto Emitter::EmitString(const std::string_view string) -> llvm::Value*
    {
        auto* const charType = llvm::Type::getInt8Ty(GetContext());

        std::vector<llvm::Constant*> chars(string.size());
        for (size_t i = 0; i < string.size(); i++)
        {
            chars.at(i) = llvm::ConstantInt::get(charType, string.at(i));
        }

        chars.push_back(llvm::ConstantInt::get(charType, 0));

        auto* const stringType = llvm::ArrayType::get(charType, chars.size());

        auto* const var = EmitGlobalVar(
            AnonymousIdent::Create("string"),
            stringType,
            true,
            llvm::ConstantArray::get(stringType, chars)
        );
        var->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

        return var;
    }

    auto Emitter::EmitPrintf(llvm::Value* const message) -> void
    {
        EmitPrintf(std::vector{ message });
    }

    auto Emitter::EmitPrintf(const std::vector<llvm::Value*>& args) -> void
    {
        GetBlock().Builder.CreateCall(GetC().GetFunctions().GetPrintf(), args);
    }

    auto Emitter::EmitPrint(const std::string_view string) -> void
    {
        return EmitPrintf(EmitString(std::string{ string }));
    }

    auto Emitter::EmitPrintLine(const std::string_view string) -> void
    {
        return EmitPrintf(EmitString(std::string{ string } + "\n"));
    }

    auto Emitter::GetCompilation() const -> Compilation*
    {
        return m_Compilation;
    }

    auto Emitter::GetContext() const -> const llvm::LLVMContext&
    {
        return m_Context;
    }

    auto Emitter::GetContext() -> llvm::LLVMContext&
    {
        return m_Context;
    }

    auto Emitter::GetModule() const -> const llvm::Module&
    {
        return *m_Module.get();
    }

    auto Emitter::GetModule() -> llvm::Module&
    {
        return *m_Module.get();
    }

    auto Emitter::GetC() const -> const C&
    {
        return m_C;
    }

    auto Emitter::GetType(const ITypeSymbol* const symbol) const -> llvm::Type*
    {
        if (symbol->IsPlaceholder())
        {
            return GetType(CreateInstantiated<ITypeSymbol>(symbol));
        }

        if (symbol->IsRef())
        {
            return GetType(GetCompilation()->GetNatives().Ptr.GetSymbol());
        }

        return m_TypeMap.at(symbol->GetUnaliasedType());
    }

    auto Emitter::GetTypeInfo(
        const ITypeSymbol* const symbol
    ) const -> llvm::Constant*
    {
        auto* const instantiatedSymbol =
            CreateInstantiated<ITypeSymbol>(symbol->GetUnaliasedType());

        return m_TypeInfoMap.at(instantiatedSymbol);
    }

    auto Emitter::GetVtbl(
        const ITypeSymbol* const traitSymbol,
        const ITypeSymbol* const typeSymbol
    ) const -> llvm::Constant*
    {
        auto* const instantiatedTraitSymbol =
            CreateInstantiated<TraitTypeSymbol>(traitSymbol);

        auto* const instantiatedTypeSymbol =
            CreateInstantiated<ITypeSymbol>(typeSymbol);

        return m_VtblMap.at(instantiatedTraitSymbol).at(instantiatedTypeSymbol);
    }

    auto Emitter::GetGlobalVar(
        const GlobalVarSymbol* const symbol
    ) const -> llvm::Constant*
    {
        return m_GlobalVarMap.at(symbol);
    }

    auto Emitter::GetFunction(
        const FunctionSymbol* const symbol
    ) const -> llvm::Function*
    {
        return m_FunctionMap.at(symbol);
    }

    auto Emitter::GetLocalVar(
        const IVarSymbol* const symbol
    ) const -> llvm::Value*
    {
        return m_LocalVarMap.at(symbol);
    }

    auto Emitter::GetLabelBlockMap() -> LabelBlockMap&
    {
        return m_LabelBlockMap;
    }

    auto Emitter::GetFunction() const -> llvm::Function*
    {
        return m_Function;
    }

    auto Emitter::GetBlock() -> EmittingBlock&
    {
        return *m_Block.get();
    }

    auto Emitter::SetBlock(std::unique_ptr<EmittingBlock> value) -> void
    {
        m_Block = std::move(value);
    }

    auto Emitter::GetPtrType() const -> llvm::PointerType*
    {
        return llvm::PointerType::get(
            const_cast<Emitter*>(this)->GetContext(),
            0
        );
    }

    auto Emitter::GetTypeInfoType() -> llvm::StructType*
    {
        return llvm::StructType::create(GetContext(), std::vector<llvm::Type*>
        {
            GetPtrType(),
            GetType(GetCompilation()->GetNatives().Int.GetSymbol()),
            llvm::ArrayType::get(GetPtrType(), 0),
        });
    }

    auto Emitter::GetDropGlueType() const -> llvm::FunctionType*
    {
        return llvm::FunctionType::get(
            GetType(GetCompilation()->GetVoidTypeSymbol()),
            { GetPtrType() },
            false
        );
    }

    auto Emitter::EmitGlobalVar(
        const std::string& name,
        llvm::Type* const type,
        const bool isConstant,
        llvm::Constant* const initializer
    ) -> llvm::GlobalVariable*
    {
        GetModule().getOrInsertGlobal(name, type);
        auto* const var = GetModule().getNamedGlobal(name);

        var->setConstant(isConstant);
        var->setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);

        if (initializer)
        {
            var->setInitializer(initializer);
        }

        return var;
    }

    auto Emitter::EmitTypeInfos(
        const std::vector<ITypeSymbol*>& symbols
    ) -> void
    {
        std::vector<TypeInfoHeader> headers{};
        std::for_each(begin(symbols), end(symbols),
        [&](ITypeSymbol* const symbol)
        {
            if (const auto optHeader = EmitTypeInfoHeader(symbol))
            {
                headers.push_back(optHeader.value());
            }
        });

        std::for_each(begin(headers), end(headers),
        [&](const TypeInfoHeader& header)
        {
            EmitTypeInfoBody(header);
        });
    }

    auto Emitter::EmitTypeInfoHeader(
        ITypeSymbol* const symbol
    ) -> std::optional<TypeInfoHeader>
    {
        auto* const concreteSymbol = dynamic_cast<IConcreteTypeSymbol*>(symbol);
        auto* const traitSymbol = dynamic_cast<TraitTypeSymbol*>(symbol);

        if (!concreteSymbol && !traitSymbol)
        {
            return std::nullopt;
        }

        const auto info = concreteSymbol ?
            EmitConcreteTypeInfoHeaderInfo(concreteSymbol) :
            EmitTraitTypeInfoHeaderInfo(traitSymbol);

        std::vector<llvm::Type*> elements
        {
            GetPtrType(),
            GetType(GetCompilation()->GetNatives().Int.GetSymbol()),
        };

        for (size_t i = 0; i < (info.TypeSymbols.size() * 2); i++)
        {
            elements.push_back(GetPtrType());
        }

        auto* const type = llvm::StructType::get(GetContext(), elements);

        auto* const var = EmitGlobalVar(
            AnonymousIdent::Create("type_info", symbol->CreateSignature()),
            type,
            true
        );
        m_TypeInfoMap[symbol] = var;

        return TypeInfoHeader
        {
            type,
            var,
            info.DropGluePtr,
            info.TypeSymbols,
            info.Vtbls,
        };
    }

    auto Emitter::EmitConcreteTypeInfoHeaderInfo(
        IConcreteTypeSymbol* const symbol
    ) -> TypeInfoHeaderInfo
    {
        const auto allImpls =
            symbol->GetScope()->FindPackageMod()->GetBodyScope()->CollectSymbols<TraitImplSymbol>();

        std::vector<TraitImplSymbol*> impls{};
        std::copy_if(
            begin(allImpls),
            end  (allImpls),
            back_inserter(impls),
            [&](TraitImplSymbol* const impl)
            {
                return impl->GetType()->GetUnaliased() == symbol;
            }
        );

        std::vector<ITypeSymbol*> typeSymbols{};
        std::transform(
            begin(impls),
            end  (impls),
            back_inserter(typeSymbols),
            [&](TraitImplSymbol* const implSymbol)
            {
                return implSymbol->GetTrait()->GetUnaliasedType();
            }
        );

        std::vector<llvm::Constant*> vtbls{};
        std::transform(
            begin(impls),
            end  (impls),
            back_inserter(vtbls),
            [&](TraitImplSymbol* const implSymbol)
            {
                return GetVtbl(implSymbol->GetTrait(), implSymbol->GetType());
            }
        );

        return TypeInfoHeaderInfo
        {
            GetDropGluePtr(symbol),
            typeSymbols,
            vtbls,
        };
    }

    auto Emitter::EmitTraitTypeInfoHeaderInfo(
        TraitTypeSymbol* const symbol
    ) -> TypeInfoHeaderInfo
    {
        const auto allImplSymbols =
            symbol->GetScope()->FindPackageMod()->GetBodyScope()->CollectSymbols<TraitImplSymbol>();

        std::vector<TraitImplSymbol*> implSymbols{};
        std::copy_if(
            begin(allImplSymbols),
            end  (allImplSymbols),
            back_inserter(implSymbols),
            [&](TraitImplSymbol* const impl)
            {
                return impl->GetTrait()->GetUnaliased() == symbol;
            }
        );

        std::vector<ITypeSymbol*> typeSymbols{};
        std::transform(
            begin(implSymbols),
            end  (implSymbols),
            back_inserter(typeSymbols),
            [&](TraitImplSymbol* const implSymbol)
            {
                return implSymbol->GetType()->GetUnaliasedType();
            }
        );

        std::vector<llvm::Constant*> vtbls{};
        std::transform(
            begin(implSymbols),
            end  (implSymbols),
            back_inserter(vtbls),
            [&](TraitImplSymbol* const implSymbol)
            {
                return GetVtbl(implSymbol->GetTrait(), implSymbol->GetType());
            }
        );

        return TypeInfoHeaderInfo
        {
            llvm::ConstantPointerNull::get(GetPtrType()),
            typeSymbols,
            vtbls
        };
    }

    auto Emitter::EmitTypeInfoBody(const TypeInfoHeader& header) -> void
    {
        std::vector<llvm::Constant*> values
        {
            header.DropGluePtr,
            llvm::ConstantInt::get(
                GetType(GetCompilation()->GetNatives().Int.GetSymbol()),
                header.TypeSymbols.size()
            ),
        };

        ACE_ASSERT(header.TypeSymbols.size() == header.Vtbls.size());
        for (size_t i = 0; i < header.TypeSymbols.size(); i++)
        {
            values.push_back(GetTypeInfo(header.TypeSymbols.at(i)));
            values.push_back(header.Vtbls.at(i));
        }

        header.Var->setInitializer(
            llvm::ConstantStruct::get(header.Type, values)
        );
    }

    static auto CollectDynDispatchableTraitPrototypeSymbols(
        TraitTypeSymbol* const traitSymbol
    ) -> std::vector<PrototypeSymbol*>
    {
        const auto allSymbols = traitSymbol->CollectPrototypes();

        std::vector<PrototypeSymbol*> symbols{};
        std::copy_if(begin(allSymbols), end(allSymbols), back_inserter(symbols),
        [](PrototypeSymbol* const prototypeSymbol)
        {
            return prototypeSymbol->IsDynDispatchable();
        });

        const auto supertraitSymbols = traitSymbol->CollectSupertraits();
        std::for_each(begin(supertraitSymbols), end(supertraitSymbols),
        [&](SupertraitSymbol* const supertraitSymbol)
        {
            auto* const supertraitTraitSymbol = supertraitSymbol->GetTrait();
            const auto supertraitPrototypeSymbols =
                CollectDynDispatchableTraitPrototypeSymbols(supertraitTraitSymbol);

            symbols.insert(
                end(symbols),
                begin(supertraitPrototypeSymbols),
                end  (supertraitPrototypeSymbols)
            );
        });

        return symbols;
    }

    auto Emitter::EmitVtbls(
        const std::vector<TraitImplSymbol*>& implSymbols
    ) -> void
    {
        std::for_each(begin(implSymbols), end(implSymbols),
        [&](TraitImplSymbol* const implSymbol)
        {
            EmitVtbl(implSymbol);
        });
    }

    auto Emitter::EmitVtbl(TraitImplSymbol* const implSymbol) -> void
    {
        const auto prototypeSymbols =
            CollectDynDispatchableTraitPrototypeSymbols(implSymbol->GetTrait());

        auto* const type =
            llvm::ArrayType::get(GetPtrType(), prototypeSymbols.size());

        std::vector<llvm::Constant*> elements{};
        for (size_t i = 0; i < prototypeSymbols.size(); i++)
        {
            auto* const functionSymbol = Scope::CollectImplOfFor(
                prototypeSymbols.at(i),
                implSymbol->GetType()
            ).value();

            elements.push_back(GetFunction(functionSymbol));
        }

        m_VtblMap[implSymbol->GetTrait()][implSymbol->GetType()] = EmitGlobalVar(
            AnonymousIdent::Create("vtbl", implSymbol->CreateSignature()),
            type,
            true,
            llvm::ConstantArray::get(type, elements)
        );
    }

    auto Emitter::EmitNativeTypes() -> void
    {
        m_TypeMap[GetCompilation()->GetVoidTypeSymbol()] =
            llvm::Type::getVoidTy(GetContext());

        const auto nativeTypeMap =
            GetCompilation()->GetNatives().CollectIRTypeSymbolMap(GetContext());

        for (auto& typeSymbolPair : nativeTypeMap)
        {
            m_TypeMap[typeSymbolPair.first] = typeSymbolPair.second;
        }
    }

    auto Emitter::EmitStructTypes(
        const std::vector<StructTypeSymbol*>& symbols
    ) -> void
    {
        std::vector<StructTypeSymbol*> nonPrimitiveSymbols{};
        std::copy_if(
            begin(symbols),
            end  (symbols),
            back_inserter(nonPrimitiveSymbols),
            [](StructTypeSymbol* const symbol)
            {
                return !symbol->IsPrimitivelyEmittable();
            }
        );

        std::for_each(begin(nonPrimitiveSymbols), end(nonPrimitiveSymbols),
        [&](StructTypeSymbol* const symbol)
        {
            m_TypeMap[symbol] = llvm::StructType::create(
                GetContext(),
                symbol->CreateSignature()
            );
        });

        std::for_each(begin(nonPrimitiveSymbols), end(nonPrimitiveSymbols),
        [&](StructTypeSymbol* const symbol)
        {
            std::vector<llvm::Type*> elements{};

            const auto fieldSymbols = symbol->CollectFields();
            std::for_each(begin(fieldSymbols), end(fieldSymbols),
            [&](FieldVarSymbol* const fieldSymbol)
            {
                auto* const type = GetType(fieldSymbol->GetType());
                elements.push_back(type);
            });

            auto* const type = m_TypeMap.at(symbol);
            static_cast<llvm::StructType*>(type)->setBody(elements);
        });
    }

    auto Emitter::EmitGlobalVars(
        const std::vector<GlobalVarSymbol*>& symbols
    ) -> void
    {
        std::for_each(begin(symbols), end(symbols),
        [&](GlobalVarSymbol* const symbol)
        {
            m_GlobalVarMap[symbol] = EmitGlobalVar(
                symbol->CreateSignature(),
                GetPtrType(),
                true,
                llvm::Constant::getNullValue(GetPtrType())
            );
        });
    }

    auto Emitter::EmitFunctions(
        const std::vector<FunctionSymbol*>& symbols
    ) -> void
    {
        ClearFunctionData();
    }

    auto Emitter::EmitFunctionHeader(
        FunctionSymbol* const symbol
    ) -> FunctionHeader
    {
        const auto paramSymbols = symbol->CollectAllParams();

        std::vector<llvm::Type*> paramTypes{};
        std::transform(
            begin(paramSymbols),
            end  (paramSymbols),
            back_inserter(paramTypes),
            [&](IParamVarSymbol* const paramSymbol)
            {
                auto* const type = GetType(paramSymbol->GetType());
                return llvm::PointerType::get(type, 0);
            }
        );

        auto* const type = llvm::FunctionType::get(
            GetType(symbol->GetType()),
            paramTypes,
            false
        );

        auto* const function = llvm::Function::Create(
            type,
            llvm::Function::ExternalLinkage,
            symbol->CreateSignature(),
            GetModule()
        );

        m_FunctionMap[symbol] = function;

        auto* const block = llvm::BasicBlock::Create(
            GetContext(),
            "",
            function
        );

        return FunctionHeader{ function, symbol, block };
    }

    auto Emitter::EmitFunctionBlock(const FunctionHeader& header) -> void
    {
        ClearFunctionData();

        m_Function = header.Function;
        m_FunctionSymbol = header.Symbol;

        auto* const rootSymbol =
            dynamic_cast<FunctionSymbol*>(m_FunctionSymbol->GetRoot());
        ACE_ASSERT(rootSymbol);

        const auto optBlock = rootSymbol->GetEmittableBlock();
        if (!optBlock.has_value())
        {
            return;
        }

        SetBlock(std::make_unique<EmittingBlock>(header.Block));
        optBlock.value()->Emit(*this);

        if (!GetBlock().IsTerminated())
        {
            GetBlock().Builder.CreateUnreachable();
        }
    }

    auto Emitter::EmitStaticCall(
        FunctionSymbol* const functionSymbol,
        const std::vector<llvm::Value*>& args
    ) -> llvm::Value*
    {
        return GetBlock().Builder.CreateCall(
            m_FunctionMap.at(functionSymbol),
            args
        );
    }

    static auto CalculatePrototypeVtblIndex(
        PrototypeSymbol* const symbol
    ) -> size_t
    {
        const auto symbols = CollectDynDispatchableTraitPrototypeSymbols(
            symbol->GetParentTrait()
        );
        
        const auto it = std::find(
            begin(symbols),
            end  (symbols),
            symbol->GetRoot()
        );

        return std::distance(begin(symbols), it);
    }

    auto Emitter::EmitDynCall(
        PrototypeSymbol* const prototypeSymbol,
        const std::vector<llvm::Value*>& args
    ) -> llvm::Value*
    {
        auto* const dataPtr = GetBlock().Builder.CreateLoad(
            GetPtrType(),
            args.front()
        );

        auto* const dataType = GetType(
            GetCompilation()->GetNatives().DynStrongPtrData.GetSymbol()
        );

        auto* const vtblPtr = GetBlock().Builder.CreateLoad(
            GetPtrType(),
            GetBlock().Builder.CreateStructGEP(dataType, dataPtr, 2)
        );

        const auto index = CalculatePrototypeVtblIndex(prototypeSymbol);
        const std::vector<llvm::Value*> indices
        {
            GetBlock().Builder.getInt32(0),
            GetBlock().Builder.getInt32(index),
        };
        auto* const vtblType = llvm::ArrayType::get(GetPtrType(), 0);
        auto* const functionPtr = GetBlock().Builder.CreateLoad(
            GetPtrType(),
            GetBlock().Builder.CreateGEP(vtblType, vtblPtr, indices)
        );

        std::vector<llvm::Type*> argTypes{};
        std::transform(
            begin(args),
            end  (args),
            back_inserter(argTypes),
            [&](llvm::Value* const arg) { return arg->getType(); }
        );

        auto* const functionType = llvm::FunctionType::get(
            GetType(prototypeSymbol->GetType()),
            argTypes,
            false
        );

        return GetBlock().Builder.CreateCall(functionType, functionPtr, args);
    }

    auto Emitter::ClearFunctionData() -> void
    {
        m_Function = nullptr;
        m_FunctionSymbol = nullptr;
        SetBlock(nullptr);

        m_LocalVarMap.clear();
        m_LabelBlockMap.Clear();
        m_StmtIndexMap.clear();
        m_LocalVarSymbolStmtIndexMap.clear();
        m_LocalVarSymbolStmtIndexPairs.clear();
    }

    auto Emitter::GetDropGluePtr(
        ITypeSymbol* const typeSymbol
    ) const -> llvm::Constant*
    {
        auto* const concreteTypeSymbol =
            dynamic_cast<IConcreteTypeSymbol*>(typeSymbol);
        if (
            !concreteTypeSymbol ||
            !concreteTypeSymbol->GetDropGlue().has_value()
            )
        {
            return llvm::ConstantPointerNull::get(
                llvm::PointerType::get(GetDropGlueType(), 0)
            );
        }

        return GetFunction(concreteTypeSymbol->GetDropGlue().value());
    }

    auto Emitter::CreateOutputFilePath(
        const std::string_view name,
        const std::string_view extension
    ) -> std::filesystem::path
    {
        std::string fileName{ name };
        if (!extension.empty())
        {
            fileName += ".";
            fileName += extension;
        }

        return GetCompilation()->GetOutputPath() / fileName;
    }
}
