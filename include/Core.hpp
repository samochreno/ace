#pragma once

#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <type_traits>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Node/Base.hpp"
#include "Node/Module.hpp"
#include "BoundNode/Base.hpp"

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
        std::for_each(t_astsBegin, t_astsEnd, [&]
        (const TASTSmartPointer& t_ast)
        {
            const auto nodes = GetAllNodes(t_ast);
            allNodes.insert(allNodes.end(), nodes.begin(), nodes.end());
        });

        return allNodes;
    }

#undef TASTSmartPointer
#undef TNodeIBase

    auto ParseAST(const std::string& t_packageName, const std::string& t_text) -> Expected<std::shared_ptr<const Node::Module>>;
    auto CreateAndDefineSymbols(const std::vector<const Node::IBase*>& t_nodes) -> Expected<void>;
    auto DefineAssociations(const std::vector<const Node::IBase*>& t_nodes) -> Expected<void>;
    auto AssertControlFlow(const std::vector<const BoundNode::IBase*>& t_nodes) -> Expected<void>;
    auto AssertCanResolveTypeSizes() -> Expected<void>;
    auto SetFunctionSymbolsBodies(const std::vector<const BoundNode::IBase*>& t_nodes) -> void;


#define TBound              std::remove_reference_t<decltype(t_createBoundFunc(T{}).Unwrap())>
#define TTypeChecked        std::remove_reference_t<decltype(t_getOrCreateTypeCheckedFunc(TBound{}).Unwrap().Value)>
#define TLowered            std::remove_reference_t<decltype(t_getOrCreateLoweredFunc(TTypeChecked{}).Unwrap().Value)>
#define TLoweredTypeChecked std::remove_reference_t<decltype(t_getOrCreateLoweredTypeCheckedFunc(TLowered{}).Unwrap().Value)>

    template<
        typename T, 
        typename TCreateBoundFunc, 
        typename TGetOrCreateTypeCheckedFunc, 
        typename TGetOrCreateLoweredFunc, 
        typename TGetOrCreateLoweredTypeCheckedFunc
    >
    auto CreateBoundAndVerifiedAST(
        const T& t_ast,
        TCreateBoundFunc&& t_createBoundFunc,
        TGetOrCreateTypeCheckedFunc&& t_getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& t_getOrCreateLoweredFunc,
        TGetOrCreateLoweredTypeCheckedFunc&& t_getOrCreateLoweredTypeCheckedFunc
    ) -> Expected<TLoweredTypeChecked>
    {
        ACE_TRY(boundAST, t_createBoundFunc(t_ast));
        ACE_TRY(mchTypeCheckedAST, t_getOrCreateTypeCheckedFunc(boundAST));
        ACE_TRY(mchLoweredAST, t_getOrCreateLoweredFunc(mchTypeCheckedAST.Value));
        ACE_TRY(mchLoweredTypeCheckedAST, t_getOrCreateLoweredTypeCheckedFunc(mchLoweredAST.Value));

        auto& finalAST = mchLoweredTypeCheckedAST.Value;
        const auto nodes = GetAllNodes(finalAST);
        
        ACE_TRY_VOID(AssertControlFlow(nodes));

        return finalAST;
    }

    template<
        typename T,
        typename TCreateBoundFunc,
        typename TGetOrCreateTypeCheckedFunc,
        typename TGetOrCreateLoweredFunc,
        typename TGetOrCreateLoweredTypeCheckedFunc
    >
    auto CreateBoundAndVerifiedASTs(
        const std::vector<T>& t_asts,
        TCreateBoundFunc&& t_createBoundFunc,
        TGetOrCreateTypeCheckedFunc&& t_getOrCreateTypeCheckedFunc,
        TGetOrCreateLoweredFunc&& t_getOrCreateLoweredFunc,
        TGetOrCreateLoweredTypeCheckedFunc&& t_getOrCreateLoweredTypeCheckedFunc
    ) -> Expected<std::vector<TLoweredTypeChecked>>
    {
        ACE_TRY(boundASTs, TransformExpectedVector(t_asts, [&]
        (const T& t_ast)
        {
            return t_createBoundFunc(t_ast);
        }));

        ACE_TRY(mchTypeCheckedASTs, TransformExpectedMaybeChangedVector(boundASTs, [&]
        (const TBound& t_ast)
        {
            return t_getOrCreateTypeCheckedFunc(t_ast);
        }));

        ACE_TRY(mchLoweredASTs, TransformExpectedMaybeChangedVector(mchTypeCheckedASTs.Value, [&]
        (const TTypeChecked& t_ast)
        {
            return t_getOrCreateLoweredFunc(t_ast);
        }));

        ACE_TRY(mchLoweredTypeCheckedASTs, TransformExpectedMaybeChangedVector(mchLoweredASTs.Value, [&]
        (const TLoweredTypeChecked& t_ast)
        {
            return t_getOrCreateLoweredTypeCheckedFunc(t_ast);
        }));

        auto& finalASTs = mchLoweredTypeCheckedASTs.Value;
        const auto nodes = GetAllNodes(finalASTs.begin(), finalASTs.end());

        ACE_TRY_VOID(AssertControlFlow(nodes));

        return finalASTs;
    }

#undef TBound
#undef TTypeChecked
#undef TLowered
#undef TLoweredTypeChecked
}
