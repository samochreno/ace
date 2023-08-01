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
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Scope.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class FunctionBoundNode : 
        public std::enable_shared_from_this<FunctionBoundNode>,
        public virtual IBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<FunctionBoundNode>,
        public virtual ITypedBoundNode<FunctionSymbol>,
        public virtual ITypeCheckableBoundNode<FunctionBoundNode>,
        public virtual ILowerableBoundNode<FunctionBoundNode>
    {
    public:
        FunctionBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            FunctionSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
            const std::optional<const std::shared_ptr<const SelfParamVarBoundNode>>& optSelf,
            const std::vector<std::shared_ptr<const NormalParamVarBoundNode>>& params,
            const std::optional<std::shared_ptr<const BlockStmtBoundNode>>& optBody
        );
        virtual ~FunctionBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const FunctionBoundNode> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const FunctionBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const FunctionBoundNode>> final;

        auto GetSymbol() const -> FunctionSymbol* final;

        auto GetBody() const -> std::optional<std::shared_ptr<const BlockStmtBoundNode>>;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        FunctionSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
        std::optional<std::shared_ptr<const SelfParamVarBoundNode>> m_OptSelf{};
        std::vector<std::shared_ptr<const NormalParamVarBoundNode>> m_Params{};
        std::optional<std::shared_ptr<const BlockStmtBoundNode>> m_OptBody{};
    };
}
