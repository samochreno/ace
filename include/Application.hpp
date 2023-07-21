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
        const Compilation* const compilation,
        const std::vector<const INode*>& nodes
    ) -> Expected<void>;
    auto DefineAssociations(
        const Compilation* const compilation,
        const std::vector<const INode*>& nodes
    ) -> Expected<void>;
    auto ValidateControlFlow(
        const Compilation* const compilation,
        const std::vector<const IBoundNode*>& nodes
    ) -> Expected<void>;
    auto BindFunctionSymbolsBodies(
        const Compilation* const compilation,
        const std::vector<const IBoundNode*>& nodes
    ) -> void;

    auto Main(const std::vector<std::string_view>& args) -> void;
    
#undef TLowered
#undef TLoweredTypeChecked

    template<typename TNodeSmartPointer>
    auto GetAllNodes(const TNodeSmartPointer& ast)
    {
        auto nodes = ast->GetChildren();
        nodes.push_back(ast.get());
        return nodes;
    }

#define TASTSmartPointer std::remove_reference_t<decltype(*TIt{})>
#define TNodeIBase std::remove_const_t<std::remove_pointer_t<std::remove_cvref_t<decltype(TASTSmartPointer{}->GetChildren().front())>>>

    template<typename TIt>
    auto GetAllNodes(TIt astsBegin, TIt astsEnd)
    {
        std::vector<const TNodeIBase*> allNodes{};
        std::for_each(astsBegin, astsEnd,
        [&](const TASTSmartPointer& ast)
        {
            const auto nodes = GetAllNodes(ast);
            allNodes.insert(allNodes.end(), nodes.begin(), nodes.end());
        });

        return allNodes;
    }

#undef TASTSmartPointer
#undef TNodeIBase
#define TTypeChecked        std::remove_reference_t<decltype(getOrCreateTypeCheckedFunc(TBound{}).Unwrap().Value)>
#define TLowered            std::remove_reference_t<decltype(getOrCreateLoweredFunc(TTypeChecked{}).Value)>

    template<
        typename TBound, 
        typename TGetOrCreateTypeCheckedFunc, 
        typename TGetOrCreateLoweredFunc
    >
    auto CreateTransformedAndVerifiedAST(
        const Compilation* const compilation,
        const TBound& boundAST,
        TGetOrCreateTypeCheckedFunc&& getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& getOrCreateLoweredFunc
    ) -> Expected<TLowered>
    {
        ACE_TRY(mchTypeCheckedAST, getOrCreateTypeCheckedFunc(boundAST));
        const auto mchLoweredAST = getOrCreateLoweredFunc(
            mchTypeCheckedAST.Value
        );

        auto& finalAST = mchLoweredAST.Value;
        const auto nodes = GetAllNodes(finalAST);
        
        ACE_TRY_VOID(ValidateControlFlow(compilation, nodes));
        BindFunctionSymbolsBodies(compilation, nodes);

        return finalAST;
    }

#define TBound std::remove_reference_t<decltype(createBoundFunc(T{}).Unwrap())>

    template<
        typename T, 
        typename TCreateBoundFunc, 
        typename TGetOrCreateTypeCheckedFunc, 
        typename TGetOrCreateLoweredFunc
    >
    auto CreateBoundTransformedAndVerifiedAST(
        const Compilation* const compilation,
        const T& ast,
        TCreateBoundFunc&& createBoundFunc,
        TGetOrCreateTypeCheckedFunc&& getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& getOrCreateLoweredFunc
    ) -> Expected<TLowered>
    {
        ACE_TRY(boundAST, createBoundFunc(ast));

        ACE_TRY(finalAST, CreateTransformedAndVerifiedAST(
            compilation,
            boundAST,
            getOrCreateTypeCheckedFunc,
            getOrCreateLoweredFunc
        ));

        return finalAST;
    }

#undef TBound
#undef TTypeChecked
}
