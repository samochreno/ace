#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    struct StructConstructionExprArg
    {
        std::string Name{};
        std::optional<std::shared_ptr<const IExprNode>> OptValue{};
    };

    class StructConstructionExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<StructConstructionExprNode>,
        public virtual IBindableNode<StructConstructionExprBoundNode>
    {
    public:
        StructConstructionExprNode(
            const std::shared_ptr<Scope>& t_scope,
            const SymbolName& t_typeName,
            std::vector<StructConstructionExprArg>&& t_args
        );
        virtual ~StructConstructionExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const StructConstructionExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const StructConstructionExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_TypeName{};
        std::vector<StructConstructionExprArg> m_Args{};
    };
}
