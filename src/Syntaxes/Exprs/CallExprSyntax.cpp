#include "Syntaxes/Exprs/CallExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Syntaxes/Exprs/SymbolLiteralExprSyntax.hpp"
#include "Syntaxes/Exprs/MemberAccessExprSyntax.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "Semas/Exprs/Calls/InstanceCallExprSema.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/CallableSymbol.hpp"

namespace Ace
{
    CallExprSyntax::CallExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr,
        const std::vector<std::shared_ptr<const IExprSyntax>>& args
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_Args{ args }
    {
    }

    auto CallExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CallExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto CallExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Expr)
            .Collect(m_Args)
            .Build();
    }

    auto CallExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const IExprSema>> argSemas{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(argSemas),
        [&](const std::shared_ptr<const IExprSyntax>& arg)
        {
            return diagnostics.Collect(arg->CreateExprSema());
        });

        std::vector<ITypeSymbol*> argTypeSymbols{};
        std::transform(
            begin(argSemas),
            end  (argSemas),
            back_inserter(argTypeSymbols),
            [](const std::shared_ptr<const IExprSema>& arg)
            {
                return arg->GetTypeInfo().Symbol;
            }
        );

        if (const auto* const symbolLiteralExpr = dynamic_cast<const SymbolLiteralExprSyntax*>(m_Expr.get()))
        {
            const auto optCallableSymbol = diagnostics.Collect(
                GetScope()->ResolveStaticSymbol<ICallableSymbol>(
                    symbolLiteralExpr->GetName(),
                    Scope::CreateArgTypes(argTypeSymbols)
                )
            );
            auto* const callableSymbol = optCallableSymbol.value_or(
                GetCompilation()->GetErrorSymbols().GetCallable()
            );

            return Diagnosed
            {
                std::make_shared<const StaticCallExprSema>(
                    GetSrcLocation(),
                    GetScope(),
                    callableSymbol,
                    argSemas
                ),
                std::move(diagnostics),
            };
        }
        
        if (const auto* const memberAccessExpr = dynamic_cast<const MemberAccessExprSyntax*>(m_Expr.get()))
        {
            const auto exprSema = diagnostics.Collect(
                memberAccessExpr->GetExpr()->CreateExprSema()
            );

            auto* const selfTypeSymbol = exprSema->GetTypeInfo().Symbol;

            std::optional<ICallableSymbol*> optCallableSymbol{};
            if (!selfTypeSymbol->IsError())
            {
                optCallableSymbol = diagnostics.Collect(
                    GetScope()->ResolveInstanceSymbol<ICallableSymbol>(
                        selfTypeSymbol,
                        memberAccessExpr->GetName(),
                        Scope::CreateArgTypes(argTypeSymbols)
                    )
                );
            }

            auto* const callableSymbol = optCallableSymbol.value_or(
                GetCompilation()->GetErrorSymbols().GetCallable()
            );

            return Diagnosed
            {
                std::make_shared<const InstanceCallExprSema>(
                    GetSrcLocation(),
                    exprSema,
                    callableSymbol,
                    argSemas
                ),
                std::move(diagnostics),
            };
        }

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        diagnostics.Add(CreateExpectedFunctionError(
            exprSema->GetSrcLocation(),
            exprSema->GetTypeInfo().Symbol
        ));

        return Diagnosed{ exprSema, std::move(diagnostics) };
    }

    auto CallExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
