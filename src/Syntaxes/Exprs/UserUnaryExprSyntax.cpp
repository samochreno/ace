#include "Syntaxes/Exprs/UserUnaryExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Semas/Exprs/UserUnaryExprSema.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    UserUnaryExprSyntax::UserUnaryExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr,
        const SrcLocation& opSrcLocation,
        const Op op
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_OpSrcLocation{ opSrcLocation },
        m_Op{ op }
    {
    }

    auto UserUnaryExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserUnaryExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnaryExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    static auto GetOpSymbol(
        ITypeSymbol* const typeSymbol,
        const Op op
    ) -> std::optional<FunctionSymbol*>
    {
        auto* const compilation = typeSymbol->GetCompilation();

        const auto& opMap = compilation->GetNatives().GetUnaryOpMap();

        const auto typeOpsMapIt = opMap.find(typeSymbol->GetUnaliasedType()); 
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
        ITypeSymbol* const typeSymbol,
        const Op op
    ) -> Diagnosed<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::optional<FunctionSymbol*> optSymbol{};
        if (!typeSymbol->IsError())
        {
            optSymbol = GetOpSymbol(typeSymbol, op);
            if (!optSymbol.has_value())
            {
                diagnostics.Add(CreateUndeclaredUnaryOpError(
                    srcLocation,
                    typeSymbol
                ));
            }
        }

        auto* const symbol = optSymbol.value_or(
            scope->GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Diagnosed{ symbol, std::move(diagnostics) };
    }

    auto UserUnaryExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const UserUnaryExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        auto* const typeSymbol = exprSema->GetTypeInfo().Symbol;

        const auto opSymbol = diagnostics.Collect(
            ResolveOpSymbol(m_OpSrcLocation, GetScope(), typeSymbol, m_Op)
        );

        return Diagnosed
        {
            std::make_shared<const UserUnaryExprSema>(
                GetSrcLocation(),
                exprSema,
                opSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto UserUnaryExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
