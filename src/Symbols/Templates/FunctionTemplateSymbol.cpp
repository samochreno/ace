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
        const std::shared_ptr<const FunctionTemplateNode>& templateNode
    ) : m_TemplateNode{ templateNode }
    {
        m_Name =
        {
            GetAST()->GetTemplateName().SrcLocation,
            SpecialIdent::CreateTemplate(GetAST()->GetTemplateName().String),
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

    auto FunctionTemplateSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::FunctionTemplate;
    }

    auto FunctionTemplateSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto FunctionTemplateSymbol::GetAST() const -> std::shared_ptr<const ITemplatableNode>
    {
        return m_TemplateNode->GetAST();
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
    ) -> Diagnosed<TemplateSymbolsInstantationResult>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto implParamNames = m_TemplateNode->CollectImplParamNames();
        const auto paramNames = m_TemplateNode->CollectParamNames();

        ACE_ASSERT(implArgs.size() == implParamNames.size());
        ACE_ASSERT(args.size() == paramNames.size());

        const auto ast = m_TemplateNode->GetConcreteAST()->CloneInScope(
            m_TemplateNode->GetScope()
        );

        diagnostics.Collect(ast->GetTemplateSelfScope()->DefineTemplateArgAliases(
            implParamNames,
            implArgs,
            paramNames,
            args
        ));

        const auto nodes = Application::GetAllNodes(ast);

        diagnostics.Collect(Application::CreateAndDefineSymbols(
            GetCompilation(),
            nodes
        ));
        diagnostics.Collect(Application::DefineAssociations(
            GetCompilation(),
            nodes
        ));

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            this,
            std::nullopt,
            implArgs,
            args
        ).Unwrap();

        return Diagnosed
        {
            TemplateSymbolsInstantationResult{ symbol, ast },
            std::move(diagnostics),
        };
    }

    auto FunctionTemplateSymbol::InstantiateSemanticsForSymbols(
        const std::shared_ptr<const ITemplatableNode>& ast
    ) -> void
    {
        const auto castedAST =
            std::dynamic_pointer_cast<const FunctionNode>(ast);

        const auto boundAST = castedAST->CreateBound();
        ACE_ASSERT(boundAST.GetDiagnostics().IsEmpty());;

        const auto finalAST = Application::CreateTransformedAndVerifiedAST(
            boundAST.Unwrap(),
            [](const std::shared_ptr<const FunctionBoundNode>& ast)
            {
                return ast->CreateTypeChecked({});
            },
            [](const std::shared_ptr<const FunctionBoundNode>& ast)
            {
                return ast->CreateLowered({});
            }
        ).Unwrap();
    }
}
