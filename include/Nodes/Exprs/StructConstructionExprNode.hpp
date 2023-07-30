#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Ident.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    struct StructConstructionExprArg
    {
        Ident Name{};
        std::optional<std::shared_ptr<const IExprNode>> OptValue{};
    };

    class StructConstructionExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<StructConstructionExprNode>,
        public virtual IBindableNode<StructConstructionExprBoundNode>
    {
    public:
        StructConstructionExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const SymbolName& typeName,
            std::vector<StructConstructionExprArg>&& args
        );
        virtual ~StructConstructionExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const StructConstructionExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const StructConstructionExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_TypeName{};
        std::vector<StructConstructionExprArg> m_Args{};
    };
}
