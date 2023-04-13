#include "Core.hpp"

#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Node/Base.hpp"
#include "Node/Module.hpp"
#include "Node/Impl.hpp"
#include "BoundNode/Base.hpp"
#include "BoundNode/Function.hpp"
#include "SymbolKind.hpp"
#include "Log.hpp"
#include "Utility.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"
#include "NativeSymbol.hpp"

namespace Ace::Core
{
    auto ParseAST(const std::string& t_packageName, const std::string& t_text) -> Expected<std::shared_ptr<const Node::Module>>
    {
        ACE_TRY(tokens, Scanning::Scanner::ScanTokens(Scanning::Kind::Language, t_text));
        ACE_TRY(ast, Parsing::Parser::ParseAST(t_packageName, std::move(tokens)));
        return ast;
    }

    auto CreateAndDefineSymbols(const std::vector<const Node::IBase*>& t_nodes) -> Expected<void>
    {
        auto symbolCreatableNodes = DynamicCastFilter<const Node::ISymbolCreatable*>(t_nodes);
        std::sort(begin(symbolCreatableNodes), end(symbolCreatableNodes), []
        (const Node::ISymbolCreatable* const t_lhs, const Node::ISymbolCreatable* const t_rhs)
        {
            const auto lhsCreationOrder = GetSymbolCreationOrder(t_lhs->GetSymbolKind());
            const auto rhsCreationOrder = GetSymbolCreationOrder(t_rhs->GetSymbolKind());

            if (lhsCreationOrder < rhsCreationOrder)
                return true;

            if (lhsCreationOrder > rhsCreationOrder)
                return false;

            const auto lhsKindSpecificCreationOrder = t_lhs->GetSymbolCreationSuborder();
            const auto rhsKindSpecificCreationOrder = t_rhs->GetSymbolCreationSuborder();

            if (lhsKindSpecificCreationOrder < rhsKindSpecificCreationOrder)
                return true;

            if (lhsKindSpecificCreationOrder > rhsKindSpecificCreationOrder)
                return false;

            return false;
        });

        ACE_TRY_VOID(TransformExpectedVector(symbolCreatableNodes, []
        (const Node::ISymbolCreatable* const t_symbolCreatableNode) -> Expected<void>
        {
            ACE_TRY(symbol, Scope::DefineSymbol(t_symbolCreatableNode));
            return ExpectedVoid;
        }));

        return ExpectedVoid;
    }

    auto DefineAssociations(const std::vector<const Node::IBase*>& t_nodes) -> Expected<void>
    {
        const auto implNodes = DynamicCastFilter<const Node::Impl*>(t_nodes);

        const auto didCreateAssociations = TransformExpectedVector(implNodes, []
        (const Node::Impl* const t_implNode) -> Expected<void>
        {
            ACE_TRY_VOID(t_implNode->DefineAssociations());
            return ExpectedVoid;
        });
        ACE_TRY_ASSERT(didCreateAssociations);

        return ExpectedVoid;
    }

    auto AssertControlFlow(const std::vector<const BoundNode::IBase*>& t_nodes) -> Expected<void>
    {
        const auto functionNodes = DynamicCastFilter<const BoundNode::Function*>(t_nodes);

        const bool didControlFlowAnalysisSucceed = std::find_if(begin(functionNodes), end(functionNodes), [&]
        (const BoundNode::Function* const t_functionNode)
        {
            if (t_functionNode->GetSymbol()->GetType()->GetUnaliased() == NativeSymbol::Void.GetSymbol())
                return false;

            if (!t_functionNode->GetBody().has_value())
                return false;

            return t_functionNode->GetBody().value()->IsEndReachableWithoutReturn();

        }) == end(functionNodes);
        ACE_TRY_ASSERT(didControlFlowAnalysisSucceed);

        return ExpectedVoid;
    }

    auto AssertCanResolveTypeSizes() -> Expected<void>
    {
        const auto typeSymbols = Scope::GetRoot()->CollectDefinedSymbolsRecursive<Symbol::Type::IBase>();

        const bool canResolveSizes = std::find_if_not(begin(typeSymbols), end(typeSymbols), []
        (const Symbol::Type::IBase* const t_typeSymbol)
        {
            return t_typeSymbol->CanResolveSize();
        }) == end(typeSymbols);
        ACE_TRY_ASSERT(canResolveSizes);

        return ExpectedVoid;
    }

    auto SetFunctionSymbolsBodies(const std::vector<const BoundNode::IBase*>& t_nodes) -> void
    {
        const auto functionNodes = DynamicCastFilter<const BoundNode::Function*>(t_nodes);
        std::for_each(begin(functionNodes), end(functionNodes), [&]
        (const BoundNode::Function* const t_functionNode)
        {
            if (!t_functionNode->GetBody().has_value())
                return;

            t_functionNode->GetSymbol()->SetBody(t_functionNode->GetBody().value());
        });
    }
}
