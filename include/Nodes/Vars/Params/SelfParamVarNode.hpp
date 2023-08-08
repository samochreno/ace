#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "Nodes/TypedNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Ident.hpp"

namespace Ace
{
    class SelfParamVarNode :
        public virtual INode,
        public virtual ITypedNode,
        public virtual ICloneableInScopeNode<SelfParamVarNode>,
        public virtual IBindableNode<SelfParamVarBoundNode>
    {
    public:
        SelfParamVarNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const TypeName& typeName
        );
        virtual ~SelfParamVarNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const SelfParamVarNode> final;
        auto CreateBound() const -> Diagnosed<std::shared_ptr<const SelfParamVarBoundNode>> final;

        auto GetName() const -> const Ident& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        TypeName m_TypeName{};
    };
}
