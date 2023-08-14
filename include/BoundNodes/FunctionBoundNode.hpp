#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class FunctionBoundNode : 
        public std::enable_shared_from_this<FunctionBoundNode>,
        public virtual IBoundNode,
        public virtual ITypeCheckableBoundNode<FunctionBoundNode>,
        public virtual ILowerableBoundNode<FunctionBoundNode>
    {
    public:
        FunctionBoundNode(
            const SrcLocation& srcLocation,
            FunctionSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
            const std::optional<const std::shared_ptr<const SelfParamVarBoundNode>>& optSelf,
            const std::vector<std::shared_ptr<const NormalParamVarBoundNode>>& params,
            const std::optional<std::shared_ptr<const BlockStmtBoundNode>>& optBody
        );
        virtual ~FunctionBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const FunctionBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const FunctionBoundNode> final;

        auto GetSymbol() const -> FunctionSymbol*;

        auto GetBody() const -> std::optional<std::shared_ptr<const BlockStmtBoundNode>>;

    private:
        SrcLocation m_SrcLocation{};
        FunctionSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
        std::optional<std::shared_ptr<const SelfParamVarBoundNode>> m_OptSelf{};
        std::vector<std::shared_ptr<const NormalParamVarBoundNode>> m_Params{};
        std::optional<std::shared_ptr<const BlockStmtBoundNode>> m_OptBody{};
    };
}
