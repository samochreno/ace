#include "Symbols/Templates/FunctionTemplateSymbol.hpp"

#include <vector>

#include "Assert.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdent.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/ImplTemplateArgAliasTypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Application.hpp"
#include "Name.hpp"

namespace Ace
{
    FunctionTemplateSymbol::FunctionTemplateSymbol(
        const FunctionTemplateNode* const templateNode
    ) : m_TemplateNode{ templateNode }
    {
        m_Name =
        {
            GetASTName().SrcLocation,
            SpecialIdent::CreateTemplate(GetASTName().String),
        };
    }

    auto FunctionTemplateSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetScope();
    }

    auto FunctionTemplateSymbol::GetName() const -> const Ident&
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

    auto FunctionTemplateSymbol::GetASTName() const -> const Ident&
    {
        return m_TemplateNode->GetAST()->GetName();
    }

    auto FunctionTemplateSymbol::SetPlaceholderSymbol(
        ISymbol* const symbol
    ) -> void
    {
        m_PlaceholderSymbol = symbol;
    }

    auto FunctionTemplateSymbol::GetPlaceholderSymbol() const -> ISymbol*
    {
        return m_PlaceholderSymbol;
    }

    auto FunctionTemplateSymbol::InstantiateSymbols(
        const std::vector<ITypeSymbol*>& implArgs,
        const std::vector<ITypeSymbol*>& args
    ) -> Expected<TemplateSymbolsInstantationResult>
    {
        const auto implParamNames = m_TemplateNode->CollectImplParamNames();
        const auto paramNames = m_TemplateNode->CollectParamNames();

        ACE_TRY_ASSERT(implArgs.size() == implParamNames.size());
        ACE_TRY_ASSERT(args.size() == paramNames.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScope(
            m_TemplateNode->GetScope()
        );

        ACE_TRY_VOID(ast->GetSelfScope()->DefineTemplateArgAliases(
            implParamNames,
            implArgs,
            paramNames,
            args
        ));

        const auto nodes = Application::GetAllNodes(ast);

        ACE_TRY_VOID(Application::CreateAndDefineSymbols(GetCompilation(), nodes));
        ACE_TRY_VOID(Application::DefineAssociations(GetCompilation(), nodes));

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            this,
            std::nullopt,
            implArgs,
            args
        ).Unwrap();

        return TemplateSymbolsInstantationResult{ symbol, ast };
    }

    auto FunctionTemplateSymbol::InstantiateSemanticsForSymbols(
        const std::shared_ptr<const INode>& ast
    ) -> void
    {
        const auto boundAST = Application::CreateBoundTransformedAndVerifiedAST(
            std::dynamic_pointer_cast<const FunctionNode>(ast),
            [](const std::shared_ptr<const FunctionNode>& ast)
            {
                return ast->CreateBound();
            },
            [](const std::shared_ptr<const FunctionBoundNode>& ast)
            {
                return ast->GetOrCreateTypeChecked({});
            },
            [](const std::shared_ptr<const FunctionBoundNode>& ast)
            {
                return ast->GetOrCreateLowered({});
            }
        ).Unwrap();
    }
}
