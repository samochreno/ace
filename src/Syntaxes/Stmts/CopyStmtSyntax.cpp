#include "Syntaxes/Stmts/CopyStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Semas/Stmts/CopyStmtSema.hpp"

namespace Ace
{
    CopyStmtSyntax::CopyStmtSyntax(
        const SrcLocation& srcLocation,
        const TypeName& typeName, 
        const std::shared_ptr<const IExprSyntax>& srcExpr,
        const std::shared_ptr<const IExprSyntax>& dstExpr
    ) : m_SrcLocation{ srcLocation },
        m_TypeName{ typeName },
        m_SrcExpr{ srcExpr },
        m_DstExpr{ dstExpr }
    {
    }

    auto CopyStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CopyStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SrcExpr->GetScope();
    }

    auto CopyStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_SrcExpr)
            .Collect(m_DstExpr)
            .Build();
    }

    auto CopyStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const CopyStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto srcExprSema = diagnostics.Collect(
            m_SrcExpr->CreateExprSema()
        );
        const auto dstExprSema = diagnostics.Collect(
            m_DstExpr->CreateExprSema()
        );

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ISizedTypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed
        {
            std::make_shared<const CopyStmtSema>(
                GetSrcLocation(),
                typeSymbol,
                srcExprSema,
                dstExprSema
            ),
            std::move(diagnostics),
        };
    }

    auto CopyStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
