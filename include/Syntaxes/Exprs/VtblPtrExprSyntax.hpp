#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/VtblPtrExprSema.hpp"
#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class VtblPtrExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<VtblPtrExprSema>
    {
    public:
        VtblPtrExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const TypeName& typeName,
            const TypeName& traitName
        );
        virtual ~VtblPtrExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const VtblPtrExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        TypeName m_TypeName{};
        TypeName m_TraitName{};
    };
}
