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
            const SourceLocation& sourceLocation,
            FunctionSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
            const std::optional<const std::shared_ptr<const SelfParamVarBoundNode>>& optSelf,
            const std::vector<std::shared_ptr<const ParamVarBoundNode>>& params,
            const std::optional<std::shared_ptr<const BlockStmtBoundNode>>& optBody
        );
        virtual ~FunctionBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const FunctionBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
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
