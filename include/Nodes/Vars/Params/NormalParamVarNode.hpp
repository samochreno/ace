#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "Nodes/TypedNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class NormalParamVarNode :
        public virtual INode,
        public virtual ITypedNode,
        public virtual ICloneableNode<NormalParamVarNode>,
        public virtual IBindableNode<ParamVarBoundNode>
    {
    public:
        NormalParamVarNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_scope,
            const Identifier& t_name,
            const TypeName& t_typeName,
            const std::vector<std::shared_ptr<const AttributeNode>>& t_attributes,
            const size_t t_index
        );
        virtual ~NormalParamVarNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const NormalParamVarNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const ParamVarBoundNode>> final;

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
        size_t m_Index{};
    };
}
