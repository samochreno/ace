#include "Symbols/Templates/FunctionTemplateSymbol.hpp"

#include <vector>
#include <string>

#include "Asserts.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/ImplTemplateArgAliasTypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Core.hpp"
#include "Name.hpp"

namespace Ace
{
    FunctionTemplateSymbol::FunctionTemplateSymbol(
        const FunctionTemplateNode* const t_templateNode
    ) : m_Name
        {
            SpecialIdentifier::CreateTemplate(
                t_templateNode->GetAST()->GetName()
            ) 
        },
        m_TemplateNode{ t_templateNode }
    {
    }

    auto FunctionTemplateSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetScope();
    }

    auto FunctionTemplateSymbol::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto FunctionTemplateSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::FunctionTemplate;
    }

    auto FunctionTemplateSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto FunctionTemplateSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_TemplateNode->GetAST()->GetAccessModifier();
    }

    auto FunctionTemplateSymbol::CollectImplParams() const -> std::vector<ImplTemplateParamTypeSymbol*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectSymbols<ImplTemplateParamTypeSymbol>();
    }

    auto FunctionTemplateSymbol::CollectParams() const -> std::vector<NormalTemplateParamTypeSymbol*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectSymbols<NormalTemplateParamTypeSymbol>();
    }

    auto FunctionTemplateSymbol::GetASTName() const -> const std::string&
    {
        return m_TemplateNode->GetAST()->GetName();
    }

    auto FunctionTemplateSymbol::SetPlaceholderSymbol(
        ISymbol* const t_symbol
    ) -> void
    {
        m_PlaceholderSymbol = t_symbol;
    }

    auto FunctionTemplateSymbol::GetPlaceholderSymbol() const -> ISymbol*
    {
        return m_PlaceholderSymbol;
    }

    auto FunctionTemplateSymbol::InstantiateSymbols(
        const std::vector<ITypeSymbol*>& t_implArgs,
        const std::vector<ITypeSymbol*>& t_args
    ) -> Expected<TemplateSymbolsInstantationResult>
    {
        const auto implParamNames = m_TemplateNode->CollectImplParamNames();
        const auto paramNames = m_TemplateNode->CollectParamNames();

        ACE_TRY_ASSERT(t_implArgs.size() == implParamNames.size());
        ACE_TRY_ASSERT(t_args.size() == paramNames.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScope(
            m_TemplateNode->GetScope()
        );

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

    auto FunctionTemplateSymbol::InstantiateSemanticsForSymbols(
        const std::shared_ptr<const INode>& t_ast
    ) -> void
    {
        const auto ast = std::dynamic_pointer_cast<const FunctionNode>(t_ast);

        const auto boundAST = Core::CreateBoundTransformedAndVerifiedAST(
            GetCompilation(),
            ast,
            [](const std::shared_ptr<const FunctionNode>& t_ast)
            {
                return t_ast->CreateBound();
            },
            [](const std::shared_ptr<const FunctionBoundNode>& t_ast)
            {
                return t_ast->GetOrCreateTypeChecked({});
            },
            [](const std::shared_ptr<const FunctionBoundNode>& t_ast)
            {
                return t_ast->GetOrCreateLowered({});
            }
        ).Unwrap();
    }
}
