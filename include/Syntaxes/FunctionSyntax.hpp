#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/ImplSelfSyntax.hpp"
#include "Syntaxes/Vars/Params/SelfParamVarSyntax.hpp"
#include "Syntaxes/Vars/Params/NormalParamVarSyntax.hpp"
#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/ConstraintSyntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class FunctionSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        FunctionSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& bodyScope,
            const AccessModifier accessModifier,
            const Ident& name,
            const TypeName& typeName,
            const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
            const std::optional<std::shared_ptr<const ImplSelfSyntax>>& optSelf,
            const std::optional<std::shared_ptr<const SelfParamVarSyntax>>& optSelfParam,
            const std::vector<std::shared_ptr<const NormalParamVarSyntax>>& params,
            const std::optional<std::shared_ptr<const BlockStmtSyntax>>& optBlock,
            const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
            const std::vector<std::shared_ptr<const ConstraintSyntax>>& constraints
        );
        virtual ~FunctionSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation&;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

        auto GetBlock() const -> const std::optional<std::shared_ptr<const BlockStmtSyntax>>&;
        
    protected:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_BodyScope{};
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const AttributeSyntax>> m_Attributes{};
        std::optional<std::shared_ptr<const ImplSelfSyntax>> m_OptSelf{};
        std::optional<std::shared_ptr<const SelfParamVarSyntax>> m_OptSelfParam{};
        std::vector<std::shared_ptr<const NormalParamVarSyntax>> m_Params{};
        std::optional<std::shared_ptr<const BlockStmtSyntax>> m_OptBlock{};
        std::vector<std::shared_ptr<const TypeParamSyntax>> m_TypeParams{};
        std::vector<std::shared_ptr<const ConstraintSyntax>> m_Constraints{};
    };
}
