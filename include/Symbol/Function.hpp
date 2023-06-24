#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>

#include "Symbol/Base.hpp"
#include "Symbol/Typed.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Templatable.hpp"
#include "Symbol/Paramized.hpp"
#include "Symbol/Var/Param/Base.hpp"
#include "Symbol/Var/Param/Normal.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "TypeInfo.hpp"
#include "Emittable.hpp"

namespace Ace::Symbol::Template
{
    class Function;
}

namespace Ace::Symbol
{
    class Function :
        public virtual Symbol::IBase,
        public virtual Symbol::ITyped,
        public virtual Symbol::ISelfScoped,
        public virtual Symbol::ITemplatable,
        public virtual Symbol::IParamized
    {
    public:
        Function(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const SymbolCategory& t_symbolCategory,
            const AccessModifier& t_accessModifier,
            Symbol::Type::IBase* const t_type
        );
        virtual ~Function() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> Symbol::Type::IBase* final;

        auto CollectParams()    const -> std::vector<Symbol::Var::Param::IBase*> final;
        auto CollectAllParams() const -> std::vector<Symbol::Var::Param::IBase*>;

        auto CollectArgTypeInfos() const -> std::vector<TypeInfo>;

        auto BindBody(const std::shared_ptr<const IEmittable<void>>& t_body) -> void;
        auto GetBody() -> std::optional<const IEmittable<void>*>;

        auto GetTemplate() const -> std::optional<Symbol::Template::Function*>;

        auto CollectTemplateArgs()     const -> std::vector<Symbol::Type::IBase*> final;
        auto CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*> final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        SymbolCategory m_SymbolCategory{};
        AccessModifier m_AccessModifier{};
        Symbol::Type::IBase* m_Type{};

        std::optional<std::shared_ptr<const IEmittable<void>>> m_OptBody{};
    };
}
