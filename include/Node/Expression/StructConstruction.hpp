#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/StructConstruction.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class StructConstruction :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::StructConstruction>,
        public virtual Node::IBindable<BoundNode::Expression::StructConstruction>
    {
    public:
        struct Argument
        {
            std::string Name{};
            std::optional<std::shared_ptr<const Node::Expression::IBase>> OptValue{};
        };

        StructConstruction(
            Scope* const t_scope,
            const SymbolName& t_typeName,
            std::vector<Argument>&& t_arguments
        ) : m_Scope{ t_scope },
            m_TypeName{ t_typeName },
            m_Arguments{ t_arguments }
        {
        }
        virtual ~StructConstruction() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::StructConstruction> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::StructConstruction>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        Scope* m_Scope{};
        SymbolName m_TypeName{};
        std::vector<Argument> m_Arguments{};
    };
}
