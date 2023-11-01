#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/SizeOfExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class SizeOfExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<SizeOfExprSema>
    {
    public:
        SizeOfExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const TypeName& typeName
        );
        virtual ~SizeOfExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const SizeOfExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        TypeName m_TypeName{};
    };
}
