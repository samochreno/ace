#include "Syntaxes/Exprs/UserBinaryExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Semas/Exprs/UserBinaryExprSema.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    UserBinaryExprSyntax::UserBinaryExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& lhsExpr,
        const std::shared_ptr<const IExprSyntax>& rhsExpr,
        const SrcLocation& opSrcLocation,
        const Op op
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSrcLocation{ opSrcLocation },
        m_Op{ op }
    {
    }

    auto UserBinaryExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserBinaryExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr)
            .Build();
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
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const typeSymbol,
        const Op op,
        const std::vector<TypeInfo>& argTypeInfos
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto argTypeSymbols = CollectTypeSymbols(argTypeInfos);

        auto* const compilation = scope->GetCompilation();

        const auto& opMap = compilation->GetNatives().GetBinaryOpMap();

        const auto typeOpsIt = opMap.find(typeSymbol->GetUnaliasedType());
        if (typeOpsIt == end(opMap))
        {
            return std::nullopt;
        }

        const auto& typeOps = typeOpsIt->second;

        const auto opSymbolIt = typeOps.find(op);
        if (opSymbolIt == end(typeOps))
        {
            return std::nullopt;
        }

        auto* const opSymbol = opSymbolIt->second;

        const bool areArgsConvertible = AreTypesConvertible(
            scope,
            argTypeInfos,
            opSymbol->CollectArgTypeInfos()
        );
        if (!areArgsConvertible)
        {
            return std::nullopt;
        }

        return opSymbol;
    }

    static auto PickOpSymbol(
        const SrcLocation& srcLocation,
        ITypeSymbol* const lhsTypeSymbol,
        ITypeSymbol* const rhsTypeSymbol,
        Expected<FunctionSymbol*> expLHSOpSymbol,
        Expected<FunctionSymbol*> expRHSOpSymbol,
        const Op op
    ) -> Diagnosed<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!expLHSOpSymbol && !expRHSOpSymbol)
        {
            diagnostics.Add(CreateUndeclaredBinaryOpError(
                srcLocation,
                lhsTypeSymbol,
                rhsTypeSymbol
            ));
            
            auto* compilation = srcLocation.Buffer->GetCompilation();
            return Diagnosed
            {
                compilation->GetErrorSymbols().GetFunction(),
                std::move(diagnostics),
            };
        }

        if (
            (expLHSOpSymbol && expRHSOpSymbol) &&
            (expLHSOpSymbol.Unwrap() != expRHSOpSymbol.Unwrap())
            )
        {
            diagnostics.Add(CreateAmbiguousBinaryOpRefError(
                srcLocation,
                lhsTypeSymbol,
                rhsTypeSymbol
            ));
        }

        const auto optOpSymbol = diagnostics.Collect(expLHSOpSymbol ?
            std::move(expLHSOpSymbol) :
            std::move(expRHSOpSymbol)
        );
        return Diagnosed{ optOpSymbol.value(), std::move(diagnostics) };
    }

    static auto ResolveAndPickOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<TypeInfo>& argTypeInfos,
        ITypeSymbol* const lhsTypeSymbol,
        ITypeSymbol* const rhsTypeSymbol,
        const Op op
    ) -> Diagnosed<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::optional<FunctionSymbol*> optOpSymbol{};
        if (!lhsTypeSymbol->IsError() && !rhsTypeSymbol->IsError())
        {
            const auto optLHSOpSymbol = ResolveOpSymbol(
                srcLocation,
                scope,
                lhsTypeSymbol,
                op,
                argTypeInfos
            );

            const auto optRHSOpSymbol = ResolveOpSymbol(
                srcLocation,
                scope,
                rhsTypeSymbol,
                op,
                argTypeInfos
            );

            if (!optLHSOpSymbol.has_value() && !optRHSOpSymbol.has_value())
            {
                diagnostics.Add(CreateUndeclaredBinaryOpError(
                    srcLocation,
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
                    srcLocation,
                    lhsTypeSymbol,
                    rhsTypeSymbol
                ));
            }

            if (optLHSOpSymbol.has_value())
            {
                optOpSymbol = optLHSOpSymbol.value();
            }
            else if (optRHSOpSymbol.has_value())
            {
                optOpSymbol = optRHSOpSymbol.value();
            }
        }

        auto* const opSymbol = optOpSymbol.value_or(
            scope->GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Diagnosed{ opSymbol, std::move(diagnostics) };
    }

    auto UserBinaryExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const UserBinaryExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto lhsExprSema = diagnostics.Collect(
            m_LHSExpr->CreateExprSema()
        );
        const auto rhsExprSema = diagnostics.Collect(
            m_RHSExpr->CreateExprSema()
        );

        const std::vector<TypeInfo> argTypeInfos
        {
            lhsExprSema->GetTypeInfo(),
            rhsExprSema->GetTypeInfo(),
        };

        auto* const lhsTypeSymbol = argTypeInfos.at(0).Symbol;
        auto* const rhsTypeSymbol = argTypeInfos.at(1).Symbol;

        const auto opSymbol = diagnostics.Collect(ResolveAndPickOpSymbol(
            m_OpSrcLocation,
            GetScope(),
            argTypeInfos,
            lhsTypeSymbol,
            rhsTypeSymbol,
            m_Op
        ));

        return Diagnosed
        {
            std::make_shared<const UserBinaryExprSema>(
                GetSrcLocation(),
                lhsExprSema,
                rhsExprSema,
                opSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto UserBinaryExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
