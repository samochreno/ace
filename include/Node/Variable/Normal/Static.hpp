#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Node/Typed.hpp"
#include "Node/Attribute.hpp"
#include "BoundNode/Variable/Normal/Static.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Error.hpp"
#include "Scope.hpp"
#include "Symbol/Kind.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Variable::Normal
{
    class Static :
        public virtual Node::IBase,
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Variable::Normal::Static>,
        public virtual Node::IBindable<BoundNode::Variable::Normal::Static>
    {
    public:
        Static(
            Scope* const t_scope,
            const std::string& t_name,
            const Name::Type& t_typeName,
            const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
            const AccessModifier& t_accessModifier
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_TypeName{ t_typeName },
            m_Attributes{ t_attributes },
            m_AccessModifier{ t_accessModifier }
        {
        }
        virtual ~Static() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Variable::Normal::Static> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Variable::Normal::Static>> final;

        auto GetName() const -> const std::string& final { return m_Name; }

        auto GetSymbolScope() const -> Scope* final { return m_Scope; }
        auto GetSymbolKind() const -> Symbol::Kind final { return Symbol::Kind::StaticVariable; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        Name::Type m_TypeName{};
        std::vector<std::shared_ptr<const Node::Attribute>> m_Attributes{};
        AccessModifier m_AccessModifier{};
    };
}
