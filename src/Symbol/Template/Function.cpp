#include "Symbol/Template/Function.hpp"

#include <vector>
#include <string>

#include "Asserts.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/TemplateParam/Impl.hpp"
#include "Symbol/Type/TemplateParam/Normal.hpp"
#include "Symbol/Type/Alias/TemplateArg/Impl.hpp"
#include "Symbol/Type/Alias/TemplateArg/Normal.hpp"
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

    auto Function::CollectImplParams() const -> std::vector<Symbol::Type::TemplateParam::Impl*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectSymbols<Symbol::Type::TemplateParam::Impl>();
    }

    auto Function::CollectParams() const -> std::vector<Symbol::Type::TemplateParam::Normal*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectSymbols<Symbol::Type::TemplateParam::Normal>();
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
        const std::vector<Symbol::Type::IBase*>& t_implArgs,
        const std::vector<Symbol::Type::IBase*>& t_args
    ) -> Expected<TemplateSymbolsInstantationResult>
    {
        const auto implParamNames = m_TemplateNode->CollectImplParamNames();
        const auto paramNames = m_TemplateNode->CollectParamNames();

        ACE_TRY_ASSERT(t_implArgs.size() == implParamNames.size());
        ACE_TRY_ASSERT(t_args.size() == paramNames.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScope(m_TemplateNode->GetScope());

        ACE_TRY_VOID(ast->GetSelfScope()->DefineTemplateArgAliases(
            implParamNames,
            t_implArgs,
            paramNames,
            t_args
        ));

        const auto nodes = Core::GetAllNodes(ast);

        ACE_TRY_VOID(Core::CreateAndDefineSymbols(GetCompilation(), nodes));
        ACE_TRY_VOID(Core::DefineAssociations(GetCompilation(), nodes));

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            this,
            std::nullopt,
            t_implArgs,
            t_args
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
