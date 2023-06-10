#pragma once

#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <type_traits>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Node/Base.hpp"
#include "Node/Module.hpp"
#include "BoundNode/Base.hpp"
#include "Emittable.hpp"
#include "Compilation.hpp"

namespace Ace::Symbol
{
    class Function;

    namespace Type
    {
        class IBase;
    }
}

namespace Ace::Core
{
    template<typename TNodeSmartPointer>
    auto GetAllNodes(const TNodeSmartPointer& t_ast)
    {
        auto nodes = t_ast->GetChildren();
        nodes.push_back(t_ast.get());
        return nodes;
    }

#define TASTSmartPointer std::remove_reference_t<decltype(*TIt{})>
#define TNodeIBase std::remove_const_t<std::remove_pointer_t<std::remove_cvref_t<decltype(TASTSmartPointer{}->GetChildren().front())>>>

    template<typename TIt>
    auto GetAllNodes(TIt t_astsBegin, TIt t_astsEnd)
    {
        std::vector<const TNodeIBase*> allNodes{};
        std::for_each(t_astsBegin, t_astsEnd,
        [&](const TASTSmartPointer& t_ast)
        {
            const auto nodes = GetAllNodes(t_ast);
            allNodes.insert(allNodes.end(), nodes.begin(), nodes.end());
        });

        return allNodes;
    }

#undef TASTSmartPointer
#undef TNodeIBase

    auto ParseAST(
        const Compilation* const t_compilation,
        const std::shared_ptr<const std::filesystem::path>& t_filePath,
        const std::vector<std::string>& t_lines
    ) -> Diagnosed<std::shared_ptr<const Node::Module>, IDiagnostic>;
    auto CreateAndDefineSymbols(
        const Compilation* const t_compilation,
        const std::vector<const Node::IBase*>& t_nodes
    ) -> Expected<void>;
    auto DefineAssociations(
        const Compilation* const t_compilation,
        const std::vector<const Node::IBase*>& t_nodes
    ) -> Expected<void>;
    auto ValidateControlFlow(
        const Compilation* const t_compilation,
        const std::vector<const BoundNode::IBase*>& t_nodes
    ) -> Expected<void>;
    auto BindFunctionSymbolsBodies(
        const Compilation* const t_compilation,
        const std::vector<const BoundNode::IBase*>& t_nodes
    ) -> void;
    auto ValidateTypeSizes(
        const Compilation* const t_compilation
    ) -> Expected<void>;
    auto GenerateAndBindGlue(
        const Compilation* const t_compilation
    ) -> void;

    auto CreateCopyGlueBody(
        const Compilation* const t_compilation,
        Symbol::Type::Struct* const t_structSymbol,
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>;
    auto CreateDropGlueBody(
        const Compilation* const t_compilation,
        Symbol::Type::Struct* const t_structSymbol,
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>;

#define TTypeChecked        std::remove_reference_t<decltype(t_getOrCreateTypeCheckedFunc(TBound{}).Unwrap().Value)>
#define TLowered            std::remove_reference_t<decltype(t_getOrCreateLoweredFunc(TTypeChecked{}).Value)>
#define TLoweredTypeChecked std::remove_reference_t<decltype(t_getOrCreateLoweredTypeCheckedFunc(TLowered{}).Unwrap().Value)>

    template<
        typename TBound, 
        typename TGetOrCreateTypeCheckedFunc, 
        typename TGetOrCreateLoweredFunc, 
        typename TGetOrCreateLoweredTypeCheckedFunc
    >
    auto CreateTransformedAndVerifiedAST(
        const Compilation* const t_compilation,
        const TBound& t_boundAST,
        TGetOrCreateTypeCheckedFunc&& t_getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& t_getOrCreateLoweredFunc,
        TGetOrCreateLoweredTypeCheckedFunc&& t_getOrCreateLoweredTypeCheckedFunc
    ) -> Expected<TLoweredTypeChecked>
    {
        ACE_TRY(mchTypeCheckedAST, t_getOrCreateTypeCheckedFunc(t_boundAST));
        const auto mchLoweredAST = t_getOrCreateLoweredFunc(mchTypeCheckedAST.Value);
        ACE_TRY(mchLoweredTypeCheckedAST, t_getOrCreateLoweredTypeCheckedFunc(mchLoweredAST.Value));

        auto& finalAST = mchLoweredTypeCheckedAST.Value;
        const auto nodes = GetAllNodes(finalAST);
        
        ACE_TRY_VOID(ValidateControlFlow(t_compilation, nodes));

        return finalAST;
    }

    template<
        typename TBound,
        typename TGetOrCreateTypeCheckedFunc,
        typename TGetOrCreateLoweredFunc,
        typename TGetOrCreateLoweredTypeCheckedFunc
    >
    auto CreateTransformedAndVerifiedASTs(
        const Compilation* const t_compilation,
        const std::vector<TBound>& t_boundASTs,
        TGetOrCreateTypeCheckedFunc&& t_getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& t_getOrCreateLoweredFunc,
        TGetOrCreateLoweredTypeCheckedFunc&& t_getOrCreateLoweredTypeCheckedFunc
    ) -> Expected<std::vector<TLoweredTypeChecked>>
    {
        ACE_TRY(mchTypeCheckedASTs, TransformExpectedMaybeChangedVector(t_boundASTs,
        [&](const TBound& t_ast)
        {
            return t_getOrCreateTypeCheckedFunc(t_ast);
        }));

        const auto mchLoweredASTs = TransformMaybeChangedVector(mchTypeCheckedASTs.Value,
        [&](const TTypeChecked& t_ast)
        {
            return t_getOrCreateLoweredFunc(t_ast);
        });

        ACE_TRY(mchLoweredTypeCheckedASTs, TransformExpectedMaybeChangedVector(mchLoweredASTs.Value,
        [&](const TLoweredTypeChecked& t_ast)
        {
            return t_getOrCreateLoweredTypeCheckedFunc(t_ast);
        }));

        auto& finalASTs = mchLoweredTypeCheckedASTs.Value;
        const auto nodes = GetAllNodes(finalASTs.begin(), finalASTs.end());

        ACE_TRY_VOID(ValidateControlFlow(t_compilation, nodes));

        return finalASTs;
    }

#define TBound std::remove_reference_t<decltype(t_createBoundFunc(T{}).Unwrap())>

    template<
        typename T, 
        typename TCreateBoundFunc, 
        typename TGetOrCreateTypeCheckedFunc, 
        typename TGetOrCreateLoweredFunc, 
        typename TGetOrCreateLoweredTypeCheckedFunc
    >
    auto CreateBoundTransformedAndVerifiedAST(
        const Compilation* const t_compilation,
        const T& t_ast,
        TCreateBoundFunc&& t_createBoundFunc,
        TGetOrCreateTypeCheckedFunc&& t_getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& t_getOrCreateLoweredFunc,
        TGetOrCreateLoweredTypeCheckedFunc&& t_getOrCreateLoweredTypeCheckedFunc
    ) -> Expected<TLoweredTypeChecked>
    {
        ACE_TRY(boundAST, t_createBoundFunc(t_ast));
        ACE_TRY(finalAST, CreateTransformedAndVerifiedAST(
            t_compilation,
            boundAST,
            t_getOrCreateTypeCheckedFunc,
            t_getOrCreateLoweredFunc,
            t_getOrCreateLoweredTypeCheckedFunc
        ));
        return finalAST;
    }

    template<
        typename T,
        typename TCreateBoundFunc,
        typename TGetOrCreateTypeCheckedFunc,
        typename TGetOrCreateLoweredFunc,
        typename TGetOrCreateLoweredTypeCheckedFunc
    >
    auto CreateBoundTransformedAndVerifiedASTs(
        const Compilation* const t_compilation,
        const std::vector<T>& t_asts,
        TCreateBoundFunc&& t_createBoundFunc,
        TGetOrCreateTypeCheckedFunc&& t_getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& t_getOrCreateLoweredFunc,
        TGetOrCreateLoweredTypeCheckedFunc&& t_getOrCreateLoweredTypeCheckedFunc
    ) -> Expected<std::vector<TLoweredTypeChecked>>
    {
        ACE_TRY(boundASTs, TransformExpectedVector(t_asts,
        [&](const T& t_ast)
        {
            return t_createBoundFunc(t_ast);
        }));

        ACE_TRY(finalASTs, CreateTransformedAndVerifiedASTs(
            t_compilation,
            boundASTs,
            t_getOrCreateTypeCheckedFunc,
            t_getOrCreateLoweredFunc,
            t_getOrCreateLoweredTypeCheckedFunc
        ));

        const auto nodes = GetAllNodes(finalASTs.begin(), finalASTs.end());

        ACE_TRY_VOID(ValidateControlFlow(t_compilation, nodes));

        return finalASTs;
    }

#undef TBound
#undef TTypeChecked
#undef TLowered
#undef TLoweredTypeChecked
}
