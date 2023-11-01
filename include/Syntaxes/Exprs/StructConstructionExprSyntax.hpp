#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Ident.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/StructConstructionExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    struct StructConstructionExprArg
    {
        Ident Name{};
        std::optional<std::shared_ptr<const IExprSyntax>> OptValue{};
    };

    class StructConstructionExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<StructConstructionExprSema>
    {
    public:
        StructConstructionExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const SymbolName& typeName,
            std::vector<StructConstructionExprArg>&& args
        );
        virtual ~StructConstructionExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const StructConstructionExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_TypeName{};
        std::vector<StructConstructionExprArg> m_Args{};
    };
}
