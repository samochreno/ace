#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/StructConstruction.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class StructConstruction :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::StructConstruction>,
        public virtual Node::IBindable<BoundNode::Expr::StructConstruction>
    {
    public:
        struct Arg
        {
            std::string Name{};
            std::optional<std::shared_ptr<const Node::Expr::IBase>> OptValue{};
        };

        StructConstruction(
            const std::shared_ptr<Scope>& t_scope,
            const SymbolName& t_typeName,
            std::vector<Arg>&& t_args
        );
        virtual ~StructConstruction() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Expr::StructConstruction> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Expr::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::StructConstruction>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_TypeName{};
        std::vector<Arg> m_Args{};
    };
}
