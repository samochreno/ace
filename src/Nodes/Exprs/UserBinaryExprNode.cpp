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

        const auto optSymbol = diagnostics.Collect(scope->ResolveStaticSymbol<FunctionSymbol>(
            name,
            Scope::CreateArgTypes(argTypeSymbols)
        ));
        if (!optSymbol.has_value())
        {
            return diagnostics;
        }

        const bool areArgsConvertible = AreTypesConvertible(
            scope,
            argTypeInfos,
            optSymbol.value()->CollectArgTypeInfos()
        );
        if (!areArgsConvertible)
        {
            return diagnostics;
        }

        return Expected{ optSymbol.value(), diagnostics };
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
        Expected<FunctionSymbol*> expLHSOpSymbol,
        Expected<FunctionSymbol*> expRHSOpSymbol
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

        const auto optOpSymbol = diagnostics.Collect(expLHSOpSymbol ?
            std::move(expLHSOpSymbol) :
            std::move(expRHSOpSymbol)
        );
        return Diagnosed{ optOpSymbol.value(), diagnostics };
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

        std::optional<FunctionSymbol*> optOpSymbol{};
        if (
            !lhsTypeSymbol->GetWithoutRef()->IsError() &&
            !rhsTypeSymbol->GetWithoutRef()->IsError()
            )
        {
            DiagnosticBag lhsOpDiagnostics{};
            const auto optLHSOpSymbol = lhsOpDiagnostics.Collect(ResolveOpSymbol(
                scope,
                CreateFullyQualifiedOpName(op, lhsTypeSymbol),
                argTypeInfos
            ));

            DiagnosticBag rhsOpDiagnostics{};
            const auto optRHSOpSymbol = rhsOpDiagnostics.Collect(ResolveOpSymbol(
                scope,
                CreateFullyQualifiedOpName(op, rhsTypeSymbol),
                argTypeInfos
            ));

            if (!optLHSOpSymbol.has_value() && !optRHSOpSymbol.has_value())
            {
                diagnostics.Add(CreateUndefinedBinaryOpRefError(
                    op,
                    lhsTypeSymbol,
                    rhsTypeSymbol
                ));
            }

            if (
                (optLHSOpSymbol.has_value() && optRHSOpSymbol.has_value()) &&
                (optLHSOpSymbol != optRHSOpSymbol)
                )
            {
                diagnostics.Add(CreateAmbiguousBinaryOpRefError(
                    op,
                    lhsTypeSymbol,
                    rhsTypeSymbol
                ));
            }

            if (optLHSOpSymbol.has_value())
            {
                optOpSymbol = optLHSOpSymbol.value();
                diagnostics.Add(lhsOpDiagnostics);
            }

            if (optRHSOpSymbol.has_value())
            {
                optOpSymbol = optRHSOpSymbol.value();
                diagnostics.Add(rhsOpDiagnostics);
            }
        }

        auto* const opSymbol = optOpSymbol.value_or(
            scope->GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Diagnosed{ opSymbol, diagnostics };
    }

    auto UserBinaryExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const UserBinaryExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto boundLHSExpr =
            diagnostics.Collect(m_LHSExpr->CreateBoundExpr());
        const auto boundRHSExpr =
            diagnostics.Collect(m_RHSExpr->CreateBoundExpr());

        const std::vector<TypeInfo> argTypeInfos
        {
            boundLHSExpr->GetTypeInfo(),
            boundRHSExpr->GetTypeInfo(),
        };

        auto* const lhsTypeSymbol = argTypeInfos.at(0).Symbol;
        auto* const rhsTypeSymbol = argTypeInfos.at(1).Symbol;

        const auto opSymbol = diagnostics.Collect(ResolveAndPickOpSymbol(
            GetScope(),
            m_Op,
            argTypeInfos,
            lhsTypeSymbol,
            rhsTypeSymbol
        ));

        return Diagnosed
        {
            std::make_shared<const UserBinaryExprBoundNode>(
                GetSrcLocation(),
                boundLHSExpr,
                boundRHSExpr,
                opSymbol
            ),
            diagnostics,
        };
    }

    auto UserBinaryExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
