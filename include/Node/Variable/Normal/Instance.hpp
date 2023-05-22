#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Node/Typed.hpp"
#include "Node/Attribute.hpp"
#include "BoundNode/Variable/Normal/Instance.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Error.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Variable::Normal
{
    class Instance :
        public virtual Node::IBase,
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Variable::Normal::Instance>,
        public virtual Node::IBindable<BoundNode::Variable::Normal::Instance>
    {
    public:
        Instance(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            const TypeName& t_typeName,
            const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
            const AccessModifier& t_accessModifier,
            const size_t& t_index
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_TypeName{ t_typeName },
            m_Attributes{ t_attributes },
            m_AccessModifier{ t_accessModifier },
            m_Index{ t_index }
        {
        }
        virtual ~Instance() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Variable::Normal::Instance> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Variable::Normal::Instance>> final;

        auto GetName() const -> const std::string& final { return m_Name; }

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::InstanceVariable; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;
    
    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const Node::Attribute>> m_Attributes{};
        AccessModifier m_AccessModifier{};
        size_t m_Index{};
    };
}
