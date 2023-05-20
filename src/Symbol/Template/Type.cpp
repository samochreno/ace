#include "Symbol/Template/Type.hpp"

#include <vector>
#include <string>

#include "Asserts.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Normal.hpp"
#include "Core.hpp"

namespace Ace::Symbol::Template
{
    auto Type::CollectImplParameters() const -> std::vector<Symbol::Type::TemplateParameter::Impl*>
    {
        return {};
    }

    auto Type::CollectParameters() const -> std::vector<Symbol::Type::TemplateParameter::Normal*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectDefinedSymbols<Symbol::Type::TemplateParameter::Normal>();
    }

    auto Type::InstantiateSymbols(
        const std::vector<Symbol::Type::IBase*>& t_implArguments,
        const std::vector<Symbol::Type::IBase*>& t_arguments
    ) -> Expected<TemplateSymbolsInstantationResult>
    {
        const auto parameterNames = m_TemplateNode->CollectParameterNames();

        ACE_TRY_ASSERT(t_arguments.size() == parameterNames.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScopeType(m_TemplateNode->GetScope());

        ACE_TRY_VOID(ast->GetSelfScope()->DefineTemplateArgumentAliases(
            {},
            {},
            parameterNames,
            t_arguments
        ));

        const auto nodes = Core::GetAllNodes(ast);

        ACE_TRY_VOID(Core::CreateAndDefineSymbols(GetCompilation(), nodes));
        ACE_TRY_VOID(Core::DefineAssociations(GetCompilation(), nodes));

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            this,
            std::nullopt,
            t_implArguments,
            t_arguments
        ).Unwrap();

        return TemplateSymbolsInstantationResult{ symbol, ast };
    }

    auto Type::InstantiateSemanticsForSymbols(
        const std::shared_ptr<const Node::IBase>& t_ast
    ) -> void
    {
        const auto ast = std::dynamic_pointer_cast<const Node::Type::IBase>(t_ast);

        const auto boundAST = Core::CreateBoundTransformedAndVerifiedAST(
            GetCompilation(),
            ast,
            [](const std::shared_ptr<const Node::Type::IBase>& t_ast) { return t_ast->CreateBoundType(); },
            [](const std::shared_ptr<const BoundNode::Type::IBase>& t_ast) { return t_ast->GetOrCreateTypeCheckedType({}); },
            [](const std::shared_ptr<const BoundNode::Type::IBase>& t_ast) { return t_ast->GetOrCreateLoweredType({}); },
            [](const std::shared_ptr<const BoundNode::Type::IBase>& t_ast) { return t_ast->GetOrCreateTypeCheckedType({}); }
        ).Unwrap();

        Core::BindFunctionSymbolsBodies(
            GetCompilation(),
            Core::GetAllNodes(boundAST)
        );

        auto* const selfSymbol = boundAST->GetSymbol();

        auto copyOperatorName = selfSymbol->CreateFullyQualifiedName();
        copyOperatorName.Sections.push_back(SymbolNameSection{ SpecialIdentifier::Operator::Copy });
        selfSymbol->GetScope()->ResolveStaticSymbol<Symbol::Function>(copyOperatorName);

        auto dropOperatorName = selfSymbol->CreateFullyQualifiedName();
        dropOperatorName.Sections.push_back(SymbolNameSection{ SpecialIdentifier::Operator::Drop });
        selfSymbol->GetScope()->ResolveStaticSymbol<Symbol::Function>(dropOperatorName);
    }
}
