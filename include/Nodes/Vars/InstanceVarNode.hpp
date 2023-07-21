#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "Nodes/TypedNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class InstanceVarNode :
        public virtual INode,
        public virtual ITypedNode,
        public virtual ICloneableNode<InstanceVarNode>,
        public virtual IBindableNode<InstanceVarBoundNode>
    {
    public:
        InstanceVarNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            const Identifier& name,
            const TypeName& typeName,
            const std::vector<std::shared_ptr<const AttributeNode>>& attributes,
            const AccessModifier accessModifier,
            const size_t index
        );
        virtual ~InstanceVarNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const InstanceVarNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const InstanceVarBoundNode>> final;

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
        std::vector<std::shared_ptr<const AttributeNode>> m_Attributes{};
        AccessModifier m_AccessModifier{};
        size_t m_Index{};
    };
}
