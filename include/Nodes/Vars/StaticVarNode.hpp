#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Node.hpp"
#include "Nodes/TypedNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Diagnostics.hpp"
#include "Scope.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class StaticVarNode :
        public virtual INode,
        public virtual ITypedNode,
        public virtual ICloneableNode<StaticVarNode>,
        public virtual IBindableNode<StaticVarBoundNode>
    {
    public:
        StaticVarNode(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            const TypeName& t_typeName,
            const std::vector<std::shared_ptr<const AttributeNode>>& t_attributes,
            const AccessModifier t_accessModifier
        );
        virtual ~StaticVarNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const StaticVarNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const StaticVarBoundNode>> final;

        auto GetName() const -> const std::string& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const AttributeNode>> m_Attributes{};
        AccessModifier m_AccessModifier{};
    };
}
