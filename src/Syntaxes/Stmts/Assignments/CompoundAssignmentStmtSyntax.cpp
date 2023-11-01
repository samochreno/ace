#include "Syntaxes/Stmts/Assignments/CompoundAssignmentStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Op.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Semas/Stmts/Assignments/CompoundAssignmentStmtSema.hpp"

namespace Ace
{
    CompoundAssignmentStmtSyntax::CompoundAssignmentStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprSyntax>& lhsExpr,
        const std::shared_ptr<const IExprSyntax>& rhsExpr,
        const SrcLocation& opSrcLocation,
        const Op op
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSrcLocation{ opSrcLocation },
        m_Op{ op }
    {
    }

    auto CompoundAssignmentStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CompoundAssignmentStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto CompoundAssignmentStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr)
            .Build();
    }

    static auto GetOpSymbol(
        ITypeSymbol* const typeSymbol,
        const Op op
    ) -> std::optional<FunctionSymbol*>
    {
        auto* const compilation = typeSymbol->GetCompilation();

        const auto& opMap = compilation->GetNatives().GetBinaryOpMap();

        const auto typeOpsMapIt = opMap.find(
            typeSymbol->GetWithoutRef()->GetUnaliasedType()
        );
        if (typeOpsMapIt == end(opMap))
        {
            return std::nullopt;
        }

        const auto& typeOpMap = typeOpsMapIt->second;

        const auto opSymbolIt = typeOpMap.find(op);
        if (opSymbolIt == end(typeOpMap))
        {
            return std::nullopt;
        }

        return opSymbolIt->second;
    }

    static auto ResolveOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const lhsTypeSymbol,
        ITypeSymbol* const rhsTypeSymbol,
        const Op op
    ) -> Diagnosed<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::optional<FunctionSymbol*> optSymbol{};
        if (!lhsTypeSymbol->IsError() && !rhsTypeSymbol->IsError())
        {
            optSymbol = GetOpSymbol(lhsTypeSymbol, op);
            if (!optSymbol.has_value())
            {
                diagnostics.Add(CreateUndeclaredBinaryOpError(
                    srcLocation,
                    lhsTypeSymbol,
                    rhsTypeSymbol
                ));
            }
        }

        auto* const symbol = optSymbol.value_or(
            scope->GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Diagnosed{ symbol, std::move(diagnostics) };
    }

    auto CompoundAssignmentStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const CompoundAssignmentStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto lhsExprSema = diagnostics.Collect(
            m_LHSExpr->CreateExprSema()
        );
        const auto rhsExprSema = diagnostics.Collect(
            m_RHSExpr->CreateExprSema()
        );

        auto* const lhsTypeSymbol = lhsExprSema->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = rhsExprSema->GetTypeInfo().Symbol;

        const auto opSymbol = diagnostics.Collect(ResolveOpSymbol(
            m_OpSrcLocation,
            GetScope(),
            lhsTypeSymbol,
            rhsTypeSymbol,
            m_Op
        ));

        return Diagnosed
        {
            std::make_shared<const CompoundAssignmentStmtSema>(
                GetSrcLocation(),
                lhsExprSema,
                rhsExprSema,
                opSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto CompoundAssignmentStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
