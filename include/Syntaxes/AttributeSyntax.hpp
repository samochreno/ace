#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/Exprs/StructConstructionExprSyntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"

namespace Ace
{
    class AttributeSyntax : public virtual ISyntax
    {
    public:
        AttributeSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const StructConstructionExprSyntax>& structConstructionExpr
        );
        virtual ~AttributeSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const StructConstructionExprSyntax> m_StructConstructionExpr{};
    };
}
