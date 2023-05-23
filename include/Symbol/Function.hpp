#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>

#include "Symbol/Base.hpp"
#include "Symbol/Typed.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Templatable.hpp"
#include "Symbol/Parameterized.hpp"
#include "Symbol/Variable/Parameter/Base.hpp"
#include "Symbol/Variable/Parameter/Normal.hpp"
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
        public virtual Symbol::IParameterized
    {
    public:
        Function(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const SymbolCategory& t_symbolCategory,
            const AccessModifier& t_accessModifier,
            Symbol::Type::IBase* const t_type
        ) : m_SelfScope{ t_selfScope },
            m_Name{ t_name },
            m_SymbolCategory{ t_symbolCategory },
            m_AccessModifier{ t_accessModifier },
            m_Type{ t_type }
        {
        }
        virtual ~Function() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_SelfScope->GetParent().value(); }
        auto GetSelfScope() const -> std::shared_ptr<Scope> final { return m_SelfScope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::Function; }
        auto GetSymbolCategory() const -> SymbolCategory final { return m_SymbolCategory; }
        auto GetAccessModifier() const -> AccessModifier final { return m_AccessModifier; }

        auto GetType() const -> Symbol::Type::IBase* final { return m_Type; }

        auto CollectParameters()    const -> std::vector<Symbol::Variable::Parameter::IBase*> final;
        auto CollectAllParameters() const -> std::vector<Symbol::Variable::Parameter::IBase*>;

        auto CollectArgumentTypeInfos() const -> std::vector<TypeInfo>;

        auto BindBody(const std::shared_ptr<const IEmittable<void>>& t_body) -> void;
        auto GetBody() -> std::optional<const IEmittable<void>*>;

        auto GetTemplate() const -> std::optional<Symbol::Template::Function*>;

        auto CollectTemplateArguments()     const -> std::vector<Symbol::Type::IBase*> final;
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        SymbolCategory m_SymbolCategory{};
        AccessModifier m_AccessModifier{};
        Symbol::Type::IBase* m_Type{};

        std::optional<std::shared_ptr<const IEmittable<void>>> m_OptBody{};
    };
}
