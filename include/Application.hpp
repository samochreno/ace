#pragma once

#include <vector>
#include <string_view>

#include "Diagnostic.hpp"
#include "Compilation.hpp"
#include "Nodes/Node.hpp"
#include "BoundNodes/BoundNode.hpp"

namespace Ace::Application
{
    auto CreateAndDefineSymbols(
        const Compilation* const t_compilation,
        const std::vector<const INode*>& t_nodes
    ) -> Expected<void>;
    auto DefineAssociations(
        const Compilation* const t_compilation,
        const std::vector<const INode*>& t_nodes
    ) -> Expected<void>;
    auto ValidateControlFlow(
        const Compilation* const t_compilation,
        const std::vector<const IBoundNode*>& t_nodes
    ) -> Expected<void>;
    auto BindFunctionSymbolsBodies(
        const Compilation* const t_compilation,
        const std::vector<const IBoundNode*>& t_nodes
    ) -> void;

    auto Main(const std::vector<std::string_view>& t_args) -> void;
    
#undef TLowered
#undef TLoweredTypeChecked

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
#define TTypeChecked        std::remove_reference_t<decltype(t_getOrCreateTypeCheckedFunc(TBound{}).Unwrap().Value)>
#define TLowered            std::remove_reference_t<decltype(t_getOrCreateLoweredFunc(TTypeChecked{}).Value)>

    template<
        typename TBound, 
        typename TGetOrCreateTypeCheckedFunc, 
        typename TGetOrCreateLoweredFunc
    >
    auto CreateTransformedAndVerifiedAST(
        const Compilation* const t_compilation,
        const TBound& t_boundAST,
        TGetOrCreateTypeCheckedFunc&& t_getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& t_getOrCreateLoweredFunc
    ) -> Expected<TLowered>
    {
        ACE_TRY(mchTypeCheckedAST, t_getOrCreateTypeCheckedFunc(t_boundAST));
        const auto mchLoweredAST = t_getOrCreateLoweredFunc(
            mchTypeCheckedAST.Value
        );

        auto& finalAST = mchLoweredAST.Value;
        const auto nodes = GetAllNodes(finalAST);
        
        ACE_TRY_VOID(ValidateControlFlow(t_compilation, nodes));
        BindFunctionSymbolsBodies(t_compilation, nodes);

        return finalAST;
    }

#define TBound std::remove_reference_t<decltype(t_createBoundFunc(T{}).Unwrap())>

    template<
        typename T, 
        typename TCreateBoundFunc, 
        typename TGetOrCreateTypeCheckedFunc, 
        typename TGetOrCreateLoweredFunc
    >
    auto CreateBoundTransformedAndVerifiedAST(
        const Compilation* const t_compilation,
        const T& t_ast,
        TCreateBoundFunc&& t_createBoundFunc,
        TGetOrCreateTypeCheckedFunc&& t_getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& t_getOrCreateLoweredFunc
    ) -> Expected<TLowered>
    {
        ACE_TRY(boundAST, t_createBoundFunc(t_ast));

        ACE_TRY(finalAST, CreateTransformedAndVerifiedAST(
            t_compilation,
            boundAST,
            t_getOrCreateTypeCheckedFunc,
            t_getOrCreateLoweredFunc
        ));

        return finalAST;
    }

#undef TBound
#undef TTypeChecked
}
