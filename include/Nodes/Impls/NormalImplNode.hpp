#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "Nodes/Impls/ImplNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class NormalImplNode :
        public virtual INode,
        public virtual IImplNode,
        public virtual ICloneableInScopeNode<NormalImplNode>,
        public virtual IBindableNode<ImplBoundNode>
    {
    public:
        NormalImplNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& selfScope,
            const SymbolName& typeName,
            const std::vector<std::shared_ptr<const FunctionNode>>& functions,
            const std::vector<std::shared_ptr<const FunctionTemplateNode>>& functionTemplates
        );
        virtual ~NormalImplNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const NormalImplNode> final;
        auto CreateBound() const -> std::shared_ptr<const ImplBoundNode> final;

        auto DefineAssociations() const -> Expected<void> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_SelfScope{};
        SymbolName m_TypeName{};
        std::vector<std::shared_ptr<const FunctionNode>> m_Functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> m_FunctionTemplates{};
    };
}
