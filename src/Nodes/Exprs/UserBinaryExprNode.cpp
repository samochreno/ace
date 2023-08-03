#include "Nodes/Exprs/UserBinaryExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"
#include "SpecialIdent.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    UserBinaryExprNode::UserBinaryExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& lhsExpr,
        const std::shared_ptr<const IExprNode>& rhsExpr,
        const Op& op
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_Op{ op }
    {
    }

    auto UserBinaryExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserBinaryExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto UserBinaryExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const UserBinaryExprNode>
    {
        return std::make_shared<const UserBinaryExprNode>(
            m_SrcLocation,
            m_LHSExpr->CloneInScopeExpr(scope),
            m_RHSExpr->CloneInScopeExpr(scope),
            m_Op
        );
    }

    auto UserBinaryExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    static auto CollectTypeSymbols(
        const std::vector<TypeInfo>& typeInfos
    ) -> std::vector<ITypeSymbol*>
    {
        std::vector<ITypeSymbol*> typeSymbols{};
        std::transform(
            begin(typeInfos),
            end  (typeInfos),
            back_inserter(typeSymbols),
            [](const auto& typeInfo) { return typeInfo.Symbol; }
        );

        return typeSymbols;
    }

    static auto ResolveOpSymbol(
        const std::shared_ptr<Scope>& scope,
        const SymbolName& name,
        const std::vector<TypeInfo>& argTypeInfos
    ) -> Expected<FunctionSymbol*>
    {
        DiagnosticBag diagnostics{};

        const auto argTypeSymbols = CollectTypeSymbols(argTypeInfos);

        const auto expSymbol = scope->ResolveStaticSymbol<FunctionSymbol>(
            name,
            Scope::CreateArgTypes(argTypeSymbols)
        );
        diagnostics.Add(expSymbol);
        if (!expSymbol)
        {
            return diagnostics;
        }

        const bool areArgsConvertible = AreTypesConvertible(
            scope,
            argTypeInfos,
            expSymbol.Unwrap()->CollectArgTypeInfos()
        );
        if (!areArgsConvertible)
        {
            return diagnostics;
        }

        return Expected{ expSymbol.Unwrap(), diagnostics };
    }

    static auto CreateFullyQualifiedOpName(
        const Op& op,
        ITypeSymbol* const typeSymbol
    ) -> SymbolName
    {
        const auto& name = SpecialIdent::Op::BinaryNameMap.at(op.TokenKind);

        auto fullyQualifiedName = typeSymbol->GetWithoutRef()->CreateFullyQualifiedName(
            op.SrcLocation
        );
        fullyQualifiedName.Sections.emplace_back(Ident{
            op.SrcLocation,
            name,
        });

        return fullyQualifiedName;
    }

    static auto PickOpSymbol(
        const Op& op,
        ITypeSymbol* const lhsTypeSymbol,
        ITypeSymbol* const rhsTypeSymbol,
        const Expected<FunctionSymbol*>& expLHSOpSymbol,
        const Expected<FunctionSymbol*>& expRHSOpSymbol
    ) -> Diagnosed<FunctionSymbol*>
    {
        DiagnosticBag diagnostics{};

        if (!expLHSOpSymbol && !expRHSOpSymbol)
        {
            diagnostics.Add(CreateUndefinedBinaryOpRefError(
                op,
                lhsTypeSymbol,
                rhsTypeSymbol
            ));
            
            auto* compilation = op.SrcLocation.Buffer->GetCompilation();
            return Diagnosed
            {
                compilation->GetErrorSymbols().GetFunction(),
                diagnostics,
            };
        }

        if (
            (expLHSOpSymbol && expRHSOpSymbol) &&
            (expLHSOpSymbol.Unwrap() != expRHSOpSymbol.Unwrap())
            )
        {
            diagnostics.Add(CreateAmbiguousBinaryOpRefError(
                op,
                lhsTypeSymbol,
                rhsTypeSymbol
            ));
        }

        if (expLHSOpSymbol)
        {
            diagnostics.Add(expLHSOpSymbol);
            return Diagnosed{ expLHSOpSymbol.Unwrap(), diagnostics };
        }
        else
        {
            diagnostics.Add(expRHSOpSymbol);
            return Diagnosed{ expRHSOpSymbol.Unwrap(), diagnostics };
        }
    }

    static auto ResolveAndPickOpSymbol(
        const std::shared_ptr<Scope>& scope,
        const Op& op,
        const std::vector<TypeInfo>& argTypeInfos,
        ITypeSymbol* const lhsTypeSymbol,
        ITypeSymbol* const rhsTypeSymbol
    ) -> Diagnosed<FunctionSymbol*>
    {
        DiagnosticBag diagnostics{};

        const auto expLHSOpSymbol = ResolveOpSymbol(
            scope,
            CreateFullyQualifiedOpName(op, lhsTypeSymbol),
            argTypeInfos
        );
        const auto expRHSOpSymbol = ResolveOpSymbol(
            scope,
            CreateFullyQualifiedOpName(op, rhsTypeSymbol),
            argTypeInfos
        );

        const auto dgnOpSymbol = PickOpSymbol(
            op,
            lhsTypeSymbol,
            rhsTypeSymbol,
            expLHSOpSymbol,
            expRHSOpSymbol
        );
        diagnostics.Add(dgnOpSymbol);

        return Diagnosed{ dgnOpSymbol.Unwrap(), diagnostics };
    }

    auto UserBinaryExprNode::CreateBound() const -> std::shared_ptr<const UserBinaryExprBoundNode>
    {
        DiagnosticBag diagnostics{};

        const auto boundLHSExpr = m_LHSExpr->CreateBoundExpr();
        const auto boundRHSExpr = m_RHSExpr->CreateBoundExpr();

        auto* const lhsTypeSymbol = boundLHSExpr->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpr->GetTypeInfo().Symbol;

        const std::vector<TypeInfo> argTypeInfos
        {
            boundLHSExpr->GetTypeInfo(),
            boundRHSExpr->GetTypeInfo(),
        };

        const auto dgnOpSymbol = ResolveAndPickOpSymbol(
            GetScope(),
            m_Op,
            argTypeInfos,
            lhsTypeSymbol,
            rhsTypeSymbol
        );
        diagnostics.Add(dgnOpSymbol);

        return std::make_shared<const UserBinaryExprBoundNode>(
            diagnostics,
            GetSrcLocation(),
            boundLHSExpr,
            boundRHSExpr,
            dgnOpSymbol.Unwrap()
        );
    }

    auto UserBinaryExprNode::CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateBound();
    }
}
