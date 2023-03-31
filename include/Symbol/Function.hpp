#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>

#include "Symbol/Base.hpp"
#include "Symbol/Typed.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Templatable.hpp"
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
    class Function : public virtual Symbol::IBase, public virtual Symbol::ITyped, public virtual Symbol::ISelfScoped, public virtual Symbol::ITemplatable
    {
    public:
        Function(
            Scope* const t_selfScope,
            const std::string& t_name,
            const AccessModifier& t_accessModifier,
            const bool& t_isInstance,
            Symbol::Type::IBase* const t_type
        ) : m_SelfScope{ t_selfScope },
            m_Name{ t_name },
            m_AccessModifier{ t_accessModifier },
            m_IsInstance{ t_isInstance },
            m_Type{ t_type }
        {
        }
        virtual ~Function() = default;

        auto GetScope() const -> Scope* final { return m_SelfScope->GetParent(); }
        auto GetSelfScope() const -> Scope* final { return m_SelfScope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> Symbol::Kind final { return Symbol::Kind::Function; }
        auto GetAccessModifier() const -> AccessModifier final { return m_AccessModifier; }
        auto IsInstance() const -> bool final { return m_IsInstance; }

        auto GetType() const -> Symbol::Type::IBase* final { return m_Type; }

        auto GetAllParameters() const -> std::vector<Symbol::Variable::Parameter::IBase*>;
        auto GetParameters() const -> std::vector<Symbol::Variable::Parameter::Normal*>;

        auto GetArgumentTypeInfos() const -> std::vector<TypeInfo>;

        auto SetBody(const std::shared_ptr<const IEmittable<void>>& t_body) -> void;
        auto GetBody() -> std::optional<const IEmittable<void>*>;

        auto SetAsNative() -> void { m_IsNative = true; }
        auto IsNative() const -> bool { return m_IsNative; }

        auto GetTemplate() const -> std::optional<Symbol::Template::Function*>;

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final;
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final;

    private:
        Scope* m_SelfScope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
        bool m_IsInstance{};
        Symbol::Type::IBase* m_Type{};

        std::optional<std::shared_ptr<const IEmittable<void>>> m_OptBody{};
        bool m_IsNative{};
    };
}
