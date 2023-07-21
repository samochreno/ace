#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "Nodes/TypedNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Identifier.hpp"

namespace Ace
{
    class SelfParamVarNode :
        public virtual INode,
        public virtual ITypedNode,
        public virtual ICloneableNode<SelfParamVarNode>,
        public virtual IBindableNode<SelfParamVarBoundNode>
    {
    public:
        SelfParamVarNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            const SymbolName& typeName
        );
        virtual ~SelfParamVarNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const SelfParamVarNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const SelfParamVarBoundNode>> final;

        auto GetName() const -> const Identifier& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Identifier m_Name{};
        TypeName m_TypeName{};
    };
}
