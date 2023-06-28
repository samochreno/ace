#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Node.hpp"
#include "Nodes/TypedNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "BoundNode/Var/Param/Self.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "SpecialIdentifier.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class SelfParamVarNode :
        public virtual INode,
        public virtual ITypedNode,
        public virtual ICloneableNode<SelfParamVarNode>,
        public virtual IBindableNode<BoundNode::Var::Param::Self>
    {
    public:
        SelfParamVarNode(
            const std::shared_ptr<Scope>& t_scope,
            const SymbolName& t_typeName
        );
        virtual ~SelfParamVarNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const SelfParamVarNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Param::Self>> final;

        auto GetName() const -> const std::string& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        TypeName m_TypeName{};
    };
}
