#include "Symbol/Template/Function.hpp"

#include <vector>
#include <string>

#include "Asserts.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/TemplateParameter/Impl.hpp"
#include "Symbol/Type/TemplateParameter/Normal.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Impl.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Normal.hpp"
#include "Symbol/Function.hpp"
#include "Core.hpp"
#include "Name.hpp"

namespace Ace::Symbol::Template
{
    Function::Function(
        const Node::Template::Function* const t_templateNode
    ) : m_Name{ SpecialIdentifier::CreateTemplate(t_templateNode->GetAST()->GetName()) },
        m_TemplateNode{ t_templateNode }
    {
    }

    auto Function::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetScope();
    }

    auto Function::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Function::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::FunctionTemplate;
    }

    auto Function::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Function::GetAccessModifier() const -> AccessModifier
    {
        return m_TemplateNode->GetAST()->GetAccessModifier();
    }

    auto Function::CollectImplParameters() const -> std::vector<Symbol::Type::TemplateParameter::Impl*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectSymbols<Symbol::Type::TemplateParameter::Impl>();
    }

    auto Function::CollectParameters() const -> std::vector<Symbol::Type::TemplateParameter::Normal*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectSymbols<Symbol::Type::TemplateParameter::Normal>();
    }

    auto Function::GetASTName() const -> const std::string&
    {
        return m_TemplateNode->GetAST()->GetName();
    }

    auto Function::SetPlaceholderSymbol(Symbol::IBase* const t_symbol) -> void
    {
        m_PlaceholderSymbol = t_symbol;
    }

    auto Function::GetPlaceholderSymbol() const -> Symbol::IBase*
    {
        return m_PlaceholderSymbol;
    }

    auto Function::InstantiateSymbols(
        const std::vector<Symbol::Type::IBase*>& t_implArguments,
        const std::vector<Symbol::Type::IBase*>& t_arguments
    ) -> Expected<TemplateSymbolsInstantationResult>
    {
        const auto implParameterNames = m_TemplateNode->CollectImplParameterNames();
        const auto parameterNames = m_TemplateNode->CollectParameterNames();

        ACE_TRY_ASSERT(t_implArguments.size() == implParameterNames.size());
        ACE_TRY_ASSERT(t_arguments.size() == parameterNames.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScope(m_TemplateNode->GetScope());

        ACE_TRY_VOID(ast->GetSelfScope()->DefineTemplateArgumentAliases(
            implParameterNames,
            t_implArguments,
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

    auto Function::InstantiateSemanticsForSymbols(
        const std::shared_ptr<const Node::IBase>& t_ast
    ) -> void
    {
        const auto ast = std::dynamic_pointer_cast<const Node::Function>(t_ast);

        const auto boundAST = Core::CreateBoundTransformedAndVerifiedAST(
            GetCompilation(),
            ast,
            [](const std::shared_ptr<const Node::Function>& t_ast) { return t_ast->CreateBound(); },
            [](const std::shared_ptr<const BoundNode::Function>& t_ast) { return t_ast->GetOrCreateTypeChecked({}); },
            [](const std::shared_ptr<const BoundNode::Function>& t_ast) { return t_ast->GetOrCreateLowered({}); },
            [](const std::shared_ptr<const BoundNode::Function>& t_ast) { return t_ast->GetOrCreateTypeChecked({}); }
        ).Unwrap();

        Core::BindFunctionSymbolsBodies(
            GetCompilation(),
            Core::GetAllNodes(boundAST)
        );
    }
}
