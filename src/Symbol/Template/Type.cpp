#include "Symbol/Template/Type.hpp"

#include <vector>
#include <string>

#include "Asserts.hpp"
#include "Scope.hpp"
#include "SpecialIdentifier.hpp"
#include "AccessModifier.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Normal.hpp"
#include "Core.hpp"

namespace Ace::Symbol::Template
{
    Type::Type(
        const Node::Template::Type* const t_templateNode
    ) : m_Name{ SpecialIdentifier::CreateTemplate(t_templateNode->GetAST()->GetName()) },
        m_TemplateNode{ t_templateNode }
    {
    }

    auto Type::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetScope();
    }

    auto Type::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetSelfScope();
    }

    auto Type::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Type::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TypeTemplate;
    }

    auto Type::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Type::GetAccessModifier() const -> AccessModifier
    {
        return m_TemplateNode->GetAST()->GetAccessModifier();
    }

    auto Type::CollectImplParameters() const -> std::vector<Symbol::Type::TemplateParameter::Impl*>
    {
        return {};
    }

    auto Type::CollectParameters() const -> std::vector<Symbol::Type::TemplateParameter::Normal*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectSymbols<Symbol::Type::TemplateParameter::Normal>();
    }

    auto Type::GetASTName() const -> const std::string&
    {
        return m_TemplateNode->GetAST()->GetName();
    }

    auto Type::SetPlaceholderSymbol(Symbol::IBase* const t_symbol) -> void
    {
        m_PlaceholderSymbol = t_symbol;
    }

    auto Type::GetPlaceholderSymbol() const -> Symbol::IBase*
    {
        return m_PlaceholderSymbol;
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
