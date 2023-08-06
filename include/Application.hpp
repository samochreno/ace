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
        Compilation* const compilation,
        const std::vector<const INode*>& nodes
    ) -> Diagnosed<void>;
    auto DefineAssociations(
        Compilation* const compilation,
        const std::vector<const INode*>& nodes
    ) -> Diagnosed<void>;
    auto ValidateControlFlow(
        Compilation* const compilation,
        const std::vector<const IBoundNode*>& nodes
    ) -> Expected<void>;
    auto BindFunctionSymbolsBodies(
        Compilation* const compilation,
        const std::vector<const IBoundNode*>& nodes
    ) -> void;

    auto Main(const std::vector<std::string_view>& args) -> void;
    
#undef TLowered
#undef TLoweredTypeChecked

    template<typename TNodeSmartPtr>
    auto GetAllNodes(const TNodeSmartPtr& ast)
    {
        auto nodes = ast->CollectChildren();
        nodes.push_back(ast.get());
        return nodes;
    }

#define TASTSmartPtr std::remove_reference_t<decltype(*TIt{})>
#define TNodeIBase std::remove_const_t<std::remove_pointer_t<std::remove_cvref_t<decltype(TASTSmartPtr{}->CollectChildren().front())>>>

    template<typename TIt>
    auto GetAllNodes(TIt astsBegin, TIt astsEnd)
    {
        std::vector<const TNodeIBase*> allNodes{};
        std::for_each(astsBegin, astsEnd,
        [&](const TASTSmartPtr& ast)
        {
            const auto nodes = GetAllNodes(ast);
            allNodes.insert(allNodes.end(), nodes.begin(), nodes.end());
        });

        return allNodes;
    }

#undef TASTSmartPtr
#undef TNodeIBase
#define TTypeChecked        std::remove_reference_t<decltype(createTypeCheckedFunc(TBound{}).Unwrap())>
#define TLowered            std::remove_reference_t<decltype(createLoweredFunc(TTypeChecked{}))>

    template<
        typename TBound, 
        typename TCreateTypeCheckedFunc, 
        typename TCreateLoweredFunc
    >
    auto CreateTransformedAndVerifiedAST(
        const TBound& boundAST,
        TCreateTypeCheckedFunc&& createTypeCheckedFunc,
        TCreateLoweredFunc&& createLoweredFunc
    ) -> Expected<TLowered>
    {
        DiagnosticBag diagnostics{};

        const auto dgnTypeCheckedAST = createTypeCheckedFunc(boundAST);
        diagnostics.Add(dgnTypeCheckedAST);

        const auto loweredAST = createLoweredFunc(dgnTypeCheckedAST.Unwrap());

        auto& finalAST = loweredAST;

        const auto nodes = GetAllNodes(finalAST);

        auto* const compilation = finalAST->GetCompilation();
        
        ACE_TRY_VOID(ValidateControlFlow(compilation, nodes));
        BindFunctionSymbolsBodies(compilation, nodes);

        return Expected{ finalAST, diagnostics };
    }

#undef TTypeChecked
#undef TLowered
}
