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
        const Op op
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
        DiagnosticBag diagnostics{};

        const auto expSymbol = scope->ResolveStaticSymbol<FunctionSymbol>(
            CreateFullyQualifiedOpName(op, typeSymbol),
            Scope::CreateArgTypes(typeSymbol)
        );
        if (!expSymbol)
        {
            diagnostics.Add(CreateUndefinedUnaryOpRefError(
                op,
                typeSymbol
            ));

            auto* const compilation = scope->GetCompilation();
            return Diagnosed
            {
                compilation->ErrorSymbols->GetFunction(),
                diagnostics,
            };
        }

        diagnostics.Add(expSymbol);
        return Diagnosed{ expSymbol.Unwrap(), diagnostics };
    }

    auto UserUnaryExprNode::CreateBound() const -> std::shared_ptr<const UserUnaryExprBoundNode>
    {
        DiagnosticBag diagnostics{};

        const auto boundExpresssion = m_Expr->CreateBoundExpr();

        auto* const typeSymbol = boundExpresssion->GetTypeInfo().Symbol;

        const auto dgnOpSymbol = ResolveOpSymbol(
            GetScope(),
            m_Op,
            typeSymbol
        );
        diagnostics.Add(dgnOpSymbol);

        return std::make_shared<const UserUnaryExprBoundNode>(
            diagnostics,
            GetSrcLocation(),
            boundExpresssion,
            dgnOpSymbol.Unwrap()
        );
    }

    auto UserUnaryExprNode::CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateBound();
    }
}
