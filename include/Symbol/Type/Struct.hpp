#pragma once

#include <vector>
#include <string>
#include <optional>

#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Variable/Normal/Instance.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Error.hpp"
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
        ) : m_SelfScope{ t_selfScope },
            m_Name{ t_name },
            m_AccessModifier{ t_accessModifier }
        {
        }
        virtual ~Struct() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_SelfScope->GetParent().value(); }
        auto GetSelfScope() const -> std::shared_ptr<Scope> final { return m_SelfScope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::Struct; }
        auto GetSymbolCategory() const -> SymbolCategory final { return SymbolCategory::Static; }
        auto GetAccessModifier() const -> AccessModifier final { return m_AccessModifier; }

        auto GetSizeKind() const -> Expected<TypeSizeKind> final;
        auto SetAsUnsized() -> void final;
        auto SetAsPrimitivelyEmittable() -> void final;
        auto IsPrimitivelyEmittable() const -> bool final;

        auto SetAsTriviallyCopyable() -> void final { m_IsTriviallyCopyable = true; }
        auto IsTriviallyCopyable() const -> bool final { return m_IsTriviallyCopyable; }
        auto SetAsTriviallyDroppable() -> void final { m_IsTriviallyDroppable = true; }
        auto IsTriviallyDroppable() const -> bool final { return m_IsTriviallyDroppable; }

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

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final { return m_SelfScope->CollectTemplateArguments(); }
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final { return m_SelfScope->CollectImplTemplateArguments(); }

        auto GetVariables() const -> std::vector<Symbol::Variable::Normal::Instance*>;

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
