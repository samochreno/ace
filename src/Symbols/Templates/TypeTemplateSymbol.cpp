#include "Symbols/Templates/TypeTemplateSymbol.hpp"

#include <vector>

#include "Assert.hpp"
#include "Scope.hpp"
#include "SpecialIdent.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Application.hpp"

namespace Ace
{
    TypeTemplateSymbol::TypeTemplateSymbol(
        const TypeTemplateNode* const templateNode
    ) : m_TemplateNode{ templateNode }
    {
        m_Name =
        {
            GetASTName().SrcLocation,
            SpecialIdent::CreateTemplate(GetASTName().String),
        };
    }

    auto TypeTemplateSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetScope();
    }

    auto TypeTemplateSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetSelfScope();
    }

    auto TypeTemplateSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto TypeTemplateSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::TypeTemplate;
    }

    auto TypeTemplateSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto TypeTemplateSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_TemplateNode->GetAST()->GetAccessModifier();
    }

    auto TypeTemplateSymbol::CollectImplParams() const -> std::vector<ImplTemplateParamTypeSymbol*>
    {
        return {};
    }

    auto TypeTemplateSymbol::CollectParams() const -> std::vector<NormalTemplateParamTypeSymbol*>
    {
        return m_TemplateNode->GetAST()->GetSelfScope()->CollectSymbols<NormalTemplateParamTypeSymbol>();
    }

    auto TypeTemplateSymbol::GetASTName() const -> const Ident&
    {
        return m_TemplateNode->GetAST()->GetName();
    }

    auto TypeTemplateSymbol::SetPlaceholderSymbol(
        ISymbol* const symbol
    ) -> void
    {
        m_PlaceholderSymbol = symbol;
    }

    auto TypeTemplateSymbol::GetPlaceholderSymbol() const -> ISymbol*
    {
        return m_PlaceholderSymbol;
    }

    auto TypeTemplateSymbol::InstantiateSymbols(
        const std::vector<ITypeSymbol*>& implArgs,
        const std::vector<ITypeSymbol*>& args
    ) -> Diagnosed<TemplateSymbolsInstantationResult>
    {
        DiagnosticBag diagnostics{};

        const auto paramNames = m_TemplateNode->CollectParamNames();

        ACE_ASSERT(args.size() == paramNames.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScopeType(
            m_TemplateNode->GetScope()
        );

        diagnostics.Collect(ast->GetSelfScope()->DefineTemplateArgAliases(
            {},
            {},
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
            diagnostics,
        };
    }

    auto TypeTemplateSymbol::InstantiateSemanticsForSymbols(
        const std::shared_ptr<const INode>& ast
    ) -> void
    {
        const auto castedAST = std::dynamic_pointer_cast<const ITypeNode>(ast);

        const auto dgnBoundAST = castedAST->CreateBoundType();
        ACE_ASSERT(dgnBoundAST.GetDiagnostics().IsEmpty());

        const auto finalAST = Application::CreateTransformedAndVerifiedAST(
            dgnBoundAST.Unwrap(),
            [](const std::shared_ptr<const ITypeBoundNode>& ast)
            {
                return ast->CreateTypeCheckedType({});
            },
            [](const std::shared_ptr<const ITypeBoundNode>& ast)
            {
                return ast->CreateLoweredType({});
            }
        ).Unwrap();

        auto* const selfSymbol = finalAST->GetSymbol();

        const SrcLocation srcLocation{};

        auto copyOpName = selfSymbol->CreateFullyQualifiedName(
            srcLocation
        );
        copyOpName.Sections.emplace_back(Ident{
            srcLocation,
            SpecialIdent::Op::Copy,
        });
        (void)selfSymbol->GetScope()->ResolveStaticSymbol<FunctionSymbol>(
            copyOpName
        );

        auto dropOpName = selfSymbol->CreateFullyQualifiedName(
            srcLocation
        );
        dropOpName.Sections.push_back(Ident{
            srcLocation,
            SpecialIdent::Op::Drop,
        });
        (void)selfSymbol->GetScope()->ResolveStaticSymbol<FunctionSymbol>(
            dropOpName
        );
    }
}
