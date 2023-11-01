#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/ConstraintSyntax.hpp"
#include "Syntaxes/FunctionSyntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Symbols/Symbol.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class InherentImplSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        InherentImplSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& bodyScope,
            const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
            const SymbolName& typeName,
            const std::vector<std::shared_ptr<const ConstraintSyntax>>& constraints,
            const std::vector<std::shared_ptr<const FunctionSyntax>>& functions
        );
        virtual ~InherentImplSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_BodyScope{};
        std::vector<std::shared_ptr<const TypeParamSyntax>> m_TypeParams{};
        SymbolName m_TypeName{};
        std::vector<std::shared_ptr<const ConstraintSyntax>> m_Constraints{};
        std::vector<std::shared_ptr<const FunctionSyntax>> m_Functions{};
    };
}
