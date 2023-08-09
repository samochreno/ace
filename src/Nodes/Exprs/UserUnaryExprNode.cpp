#include "Nodes/Exprs/UserUnaryExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "SpecialIdent.hpp"
#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"
#include "SpecialIdent.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    UserUnaryExprNode::UserUnaryExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr,
        const Op& op
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_Op{ op }
    {
    }

    auto UserUnaryExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserUnaryExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnaryExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UserUnaryExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const UserUnaryExprNode>
    {
        return std::make_unique<UserUnaryExprNode>(
            m_SrcLocation,
            m_Expr->CloneInScopeExpr(scope),
            m_Op
        );
    }

    auto UserUnaryExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    static auto CreateFullyQualifiedOpName(
        const Op& op,
        ITypeSymbol* const typeSymbol
    ) -> SymbolName
    {
        const auto& name = SpecialIdent::Op::UnaryNameMap.at(op.TokenKind);

        auto fullyQualifiedName = typeSymbol->GetWithoutRef()->CreateFullyQualifiedName(
            op.SrcLocation
        );
        fullyQualifiedName.Sections.emplace_back(Ident{
            op.SrcLocation,
            name,
        });

        return fullyQualifiedName;
    }

    static auto ResolveOpSymbol(
        const std::shared_ptr<Scope>& scope,
        const Op& op,
        ITypeSymbol* const typeSymbol
    ) -> Diagnosed<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::optional<FunctionSymbol*> optSymbol{};
        if (!typeSymbol->IsError())
        {
            auto opDiagnostics = DiagnosticBag::Create();
            optSymbol = opDiagnostics.Collect(scope->ResolveStaticSymbol<FunctionSymbol>(
                CreateFullyQualifiedOpName(op, typeSymbol),
                Scope::CreateArgTypes(typeSymbol)
            ));
            if (optSymbol.has_value())
            {
                diagnostics.Add(std::move(opDiagnostics));
            }
            else
            {
                diagnostics.Add(CreateUndefinedUnaryOpError(
                    op,
                    typeSymbol
                ));
            }
        }

        auto* const symbol = optSymbol.value_or(
            scope->GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Diagnosed{ symbol, std::move(diagnostics) };
    }

    auto UserUnaryExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const UserUnaryExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundExpr = diagnostics.Collect(m_Expr->CreateBoundExpr());

        auto* const typeSymbol = boundExpr->GetTypeInfo().Symbol;

        const auto opSymbol = diagnostics.Collect(ResolveOpSymbol(
            GetScope(),
            m_Op,
            typeSymbol
        ));

        return Diagnosed
        {
            std::make_shared<const UserUnaryExprBoundNode>(
                GetSrcLocation(),
                boundExpr,
                opSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto UserUnaryExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
