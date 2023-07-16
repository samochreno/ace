#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class FunctionBoundNode : 
        public std::enable_shared_from_this<FunctionBoundNode>,
        public virtual IBoundNode,
        public virtual ITypedBoundNode<FunctionSymbol>,
        public virtual ITypeCheckableBoundNode<FunctionBoundNode>,
        public virtual ILowerableBoundNode<FunctionBoundNode>
    {
    public:
        FunctionBoundNode(
            const SourceLocation& t_sourceLocation,
            FunctionSymbol* const t_symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& t_attributes,
            const std::optional<const std::shared_ptr<const SelfParamVarBoundNode>>& t_optSelf,
            const std::vector<std::shared_ptr<const ParamVarBoundNode>>& t_params,
            const std::optional<std::shared_ptr<const BlockStmtBoundNode>>& t_optBody
        );
        virtual ~FunctionBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const FunctionBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const FunctionBoundNode>> final;

        auto GetSymbol() const -> FunctionSymbol* final;

        auto GetBody() const -> std::optional<std::shared_ptr<const BlockStmtBoundNode>>;

    private:
        SourceLocation m_SourceLocation{};
        FunctionSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
        std::optional<std::shared_ptr<const SelfParamVarBoundNode>> m_OptSelf{};
        std::vector<std::shared_ptr<const ParamVarBoundNode>> m_Params{};
        std::optional<std::shared_ptr<const BlockStmtBoundNode>> m_OptBody{};
    };
}
