#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Var/Normal/Instance.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Diagnostics.hpp"
#include "Emittable.hpp"

namespace Ace::Symbol::Type
{
    class Struct : public virtual Symbol::Type::IBase
    {
    public:
        Struct(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const AccessModifier& t_accessModifier
        );
        virtual ~Struct() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetSizeKind() const -> Expected<TypeSizeKind> final;
        auto SetAsUnsized() -> void final;
        auto SetAsPrimitivelyEmittable() -> void final;
        auto IsPrimitivelyEmittable() const -> bool final;

        auto SetAsTriviallyCopyable() -> void final;
        auto IsTriviallyCopyable() const -> bool final;
        auto SetAsTriviallyDroppable() -> void final;
        auto IsTriviallyDroppable() const -> bool final;

        auto CreateCopyGlueBody(
            Symbol::Function* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;
        auto CreateDropGlueBody(
            Symbol::Function* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;

        auto BindCopyGlue(Symbol::Function* const t_glue) -> void final;
        auto GetCopyGlue() const -> std::optional<Symbol::Function*> final;
        auto BindDropGlue(Symbol::Function* const t_glue) -> void final;
        auto GetDropGlue() const -> std::optional<Symbol::Function*> final;

        auto CollectTemplateArgs() const -> std::vector<Symbol::Type::IBase*> final;
        auto CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*> final;

        auto GetVars() const -> std::vector<Symbol::Var::Normal::Instance*>;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};

        mutable bool m_IsResolvingSize{};
        mutable std::optional<Expected<TypeSizeKind>> m_OptSizeKindCache{};
        bool m_IsUnsized{};
        bool m_IsPrimitivelyEmittable{};
        
        bool m_IsTriviallyCopyable{};
        bool m_IsTriviallyDroppable{};

        std::optional<Symbol::Function*> m_OptCopyGlue{};
        std::optional<Symbol::Function*> m_OptDropGlue{};
    };
}
