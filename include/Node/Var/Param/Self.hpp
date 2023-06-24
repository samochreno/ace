#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Node/Typed.hpp"
#include "Node/Attribute.hpp"
#include "BoundNode/Var/Param/Self.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "SpecialIdentifier.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Var::Param
{
    class Self :
        public virtual Node::IBase,
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Var::Param::Self>,
        public virtual Node::IBindable<BoundNode::Var::Param::Self>
    {
    public:
        Self(
            const std::shared_ptr<Scope>& t_scope,
            const SymbolName& t_typeName
        );
        virtual ~Self() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Var::Param::Self> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Param::Self>> final;

        auto GetName() const -> const std::string& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        TypeName m_TypeName{};
    };
}
