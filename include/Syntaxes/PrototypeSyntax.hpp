#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/Vars/Params/SelfParamVarSyntax.hpp"
#include "Syntaxes/Vars/Params/NormalParamVarSyntax.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/ConstraintSyntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class PrototypeSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        PrototypeSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& bodyScope,
            const SymbolName& parentTraitName,
            const Ident& name,
            const TypeName& typeName,
            const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
            const size_t index,
            const std::optional<std::shared_ptr<const SelfParamVarSyntax>>& optSelfParam,
            const std::vector<std::shared_ptr<const NormalParamVarSyntax>>& params,
            const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
            const std::vector<std::shared_ptr<const ConstraintSyntax>>& constraints
        );
        virtual ~PrototypeSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation&;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_BodyScope{};
        SymbolName m_ParentTraitName{};
        Ident m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const AttributeSyntax>> m_Attributes{};
        size_t m_Index{};
        std::optional<std::shared_ptr<const SelfParamVarSyntax>> m_OptSelfParam{};
        std::vector<std::shared_ptr<const NormalParamVarSyntax>> m_Params{};
        std::vector<std::shared_ptr<const TypeParamSyntax>> m_TypeParams{};
        std::vector<std::shared_ptr<const ConstraintSyntax>> m_Constraints{};
    };
}
