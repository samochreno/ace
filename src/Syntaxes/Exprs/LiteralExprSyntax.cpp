#include "Syntaxes/Exprs/LiteralExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/LiteralExprSema.hpp"

namespace Ace
{
    LiteralExprSyntax::LiteralExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const LiteralKind kind,
        const std::string& string
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Kind{ kind },
        m_String{ string }
    {
    }

    auto LiteralExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LiteralExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LiteralExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto LiteralExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const LiteralExprSema>>
    {
        return Diagnosed
        {
            std::make_shared<const LiteralExprSema>(
                GetSrcLocation(),
                GetScope(),
                m_Kind,
                m_String
            ),
            DiagnosticBag::Create(),
        };
    }

    auto LiteralExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
