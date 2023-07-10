#include "Symbols/Templates/TypeTemplateSymbol.hpp"

#include <vector>
#include <string>

#include "Assert.hpp"
#include "Scope.hpp"
#include "SpecialIdentifier.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"
#include "Application.hpp"

namespace Ace
{
    TypeTemplateSymbol::TypeTemplateSymbol(
        const TypeTemplateNode* const t_templateNode
    ) : m_TemplateNode{ t_templateNode }
    {
        m_Name = SpecialIdentifier::CreateTemplate(GetASTName());
    }

    auto TypeTemplateSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetScope();
    }

    auto TypeTemplateSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_TemplateNode->GetSelfScope();
    }

    auto TypeTemplateSymbol::GetName() const -> const std::string&
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

    auto TypeTemplateSymbol::GetASTName() const -> const std::string&
    {
        return m_TemplateNode->GetAST()->GetName().String;
    }

    auto TypeTemplateSymbol::SetPlaceholderSymbol(
        ISymbol* const t_symbol
    ) -> void
    {
        m_PlaceholderSymbol = t_symbol;
    }

    auto TypeTemplateSymbol::GetPlaceholderSymbol() const -> ISymbol*
    {
        return m_PlaceholderSymbol;
    }

    auto TypeTemplateSymbol::InstantiateSymbols(
        const std::vector<ITypeSymbol*>& t_implArgs,
        const std::vector<ITypeSymbol*>& t_args
    ) -> Expected<TemplateSymbolsInstantationResult>
    {
        const auto paramNames = m_TemplateNode->CollectParamNames();

        ACE_TRY_ASSERT(t_args.size() == paramNames.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScopeType(
            m_TemplateNode->GetScope()
        );

        ACE_TRY_VOID(ast->GetSelfScope()->DefineTemplateArgAliases(
            {},
            {},
            paramNames,
            t_args
        ));

        const auto nodes = Application::GetAllNodes(ast);

        ACE_TRY_VOID(Application::CreateAndDefineSymbols(GetCompilation(), nodes));
        ACE_TRY_VOID(Application::DefineAssociations(GetCompilation(), nodes));

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            this,
            std::nullopt,
            t_implArgs,
            t_args
        ).Unwrap();

        return TemplateSymbolsInstantationResult{ symbol, ast };
    }

    auto TypeTemplateSymbol::InstantiateSemanticsForSymbols(
        const std::shared_ptr<const INode>& t_ast
    ) -> void
    {
        const auto ast = std::dynamic_pointer_cast<const ITypeNode>(t_ast);

        const auto boundAST = Application::CreateBoundTransformedAndVerifiedAST(
            GetCompilation(),
            ast,
            [](const std::shared_ptr<const ITypeNode>& t_ast)
            {
                return t_ast->CreateBoundType();
            },
            [](const std::shared_ptr<const ITypeBoundNode>& t_ast)
            {
                return t_ast->GetOrCreateTypeCheckedType({});
            },
            [](const std::shared_ptr<const ITypeBoundNode>& t_ast)
            {
                return t_ast->GetOrCreateLoweredType({});
            }
        ).Unwrap();

        auto* const selfSymbol = boundAST->GetSymbol();

        auto copyOperatorName = selfSymbol->CreateFullyQualifiedName();
        copyOperatorName.Sections.push_back(
            SymbolNameSection{ SpecialIdentifier::Operator::Copy }
        );
        selfSymbol->GetScope()->ResolveStaticSymbol<FunctionSymbol>(
            copyOperatorName
        );

        auto dropOperatorName = selfSymbol->CreateFullyQualifiedName();
        dropOperatorName.Sections.push_back(
            SymbolNameSection{ SpecialIdentifier::Operator::Drop }
        );
        selfSymbol->GetScope()->ResolveStaticSymbol<FunctionSymbol>(
            dropOperatorName
        );
    }
}
