#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Types/TypeNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Nodes/Vars/InstanceVarNode.hpp"
#include "BoundNodes/Types/StructTypeBoundNode.hpp"
#include "Diagnostics.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class StructTypeNode :
        public virtual ITypeNode,
        public virtual ICloneableNode<StructTypeNode>,
        public virtual IBindableNode<StructTypeBoundNode>
    {
    public:
        StructTypeNode(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const std::vector<std::shared_ptr<const AttributeNode>>& t_attributes,
            const AccessModifier t_accessModifier,
            const std::vector<std::shared_ptr<const InstanceVarNode>>& t_variables
        );
        virtual ~StructTypeNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const StructTypeNode> final;
        auto CloneInScopeType(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const ITypeNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const StructTypeBoundNode>> final;
        auto CreateBoundType() const -> Expected<std::shared_ptr<const ITypeBoundNode>> final;

        auto GetName() const -> const std::string& final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;
        
    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        std::vector<std::shared_ptr<const AttributeNode>> m_Attributes{};
        AccessModifier m_AccessModifier{};
        std::vector<std::shared_ptr<const InstanceVarNode>> m_Vars{};
    };
}
