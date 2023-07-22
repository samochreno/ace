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

    auto TypeTemplateSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TypeTemplate;
    }

    auto TypeTemplateSymbol::GetSymbolCategory() const -> SymbolCategory
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
    ) -> Expected<TemplateSymbolsInstantationResult>
    {
        const auto paramNames = m_TemplateNode->CollectParamNames();

        ACE_TRY_ASSERT(args.size() == paramNames.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScopeType(
            m_TemplateNode->GetScope()
        );

        ACE_TRY_VOID(ast->GetSelfScope()->DefineTemplateArgAliases(
            {},
            {},
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

    auto TypeTemplateSymbol::InstantiateSemanticsForSymbols(
        const std::shared_ptr<const INode>& ast
    ) -> void
    {
        const auto boundAST = Application::CreateBoundTransformedAndVerifiedAST(
            GetCompilation(),
            std::dynamic_pointer_cast<const ITypeNode>(ast),
            [](const std::shared_ptr<const ITypeNode>& ast)
            {
                return ast->CreateBoundType();
            },
            [](const std::shared_ptr<const ITypeBoundNode>& ast)
            {
                return ast->GetOrCreateTypeCheckedType({});
            },
            [](const std::shared_ptr<const ITypeBoundNode>& ast)
            {
                return ast->GetOrCreateLoweredType({});
            }
        ).Unwrap();

        auto* const selfSymbol = boundAST->GetSymbol();

        const SrcLocation srcLocation{};

        auto copyOpName = selfSymbol->CreateFullyQualifiedName(
            srcLocation
        );
        copyOpName.Sections.push_back(SymbolNameSection{
            Ident{ srcLocation, SpecialIdent::Op::Copy }
        });
        selfSymbol->GetScope()->ResolveStaticSymbol<FunctionSymbol>(
            copyOpName
        );

        auto dropOpName = selfSymbol->CreateFullyQualifiedName(
            srcLocation
        );
        dropOpName.Sections.push_back(SymbolNameSection{
            Ident{ srcLocation, SpecialIdent::Op::Drop }
        });
        selfSymbol->GetScope()->ResolveStaticSymbol<FunctionSymbol>(
            dropOpName
        );
    }
}
