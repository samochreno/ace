#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Node/Typed.hpp"
#include "Node/Attribute.hpp"
#include "BoundNode/Variable/Parameter/Normal.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Error.hpp"
#include "SymbolKind.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Variable::Parameter
{
    class Normal :
        public virtual Node::IBase,
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Variable::Parameter::Normal>,
        public virtual Node::IBindable<BoundNode::Variable::Parameter::Normal>
    {
    public:
        Normal(
            Scope* const t_scope,
            const std::string& t_name,
            const Name::Type& t_typeName,
            const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
            const size_t& t_index
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_TypeName{ t_typeName },
            m_Attributes{ t_attributes },
            m_Index{ t_index }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Variable::Parameter::Normal> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Variable::Parameter::Normal>> final;

        auto GetName() const -> const std::string& final { return m_Name; }

        auto GetSymbolScope() const -> Scope* final { return m_Scope; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::ParameterVariable; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        Name::Type m_TypeName{};
        std::vector<std::shared_ptr<const Node::Attribute>> m_Attributes{};
        size_t m_Index{};
    };
}
