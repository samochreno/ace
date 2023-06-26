#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Node/Typed.hpp"
#include "Node/Attribute.hpp"
#include "BoundNode/Var/Param/Normal.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace::Node::Var::Param
{
    class Normal :
        public virtual Node::IBase,
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Var::Param::Normal>,
        public virtual Node::IBindable<BoundNode::Var::Param::Normal>
    {
    public:
        Normal(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            const TypeName& t_typeName,
            const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
            const size_t& t_index
        );
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Var::Param::Normal> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Param::Normal>> final;

        auto GetName() const -> const std::string& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const Node::Attribute>> m_Attributes{};
        size_t m_Index{};
    };
}
