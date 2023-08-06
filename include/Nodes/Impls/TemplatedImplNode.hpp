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
#include "Symbols/Symbol.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class TemplatedImplNode :
        public virtual INode,
        public virtual IImplNode,
        public virtual ICloneableInScopeNode<TemplatedImplNode>,
        public virtual IBindableNode<ImplBoundNode>
    {
    public:
        TemplatedImplNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& selfScope,
            const SymbolName& typeTemplateName,
            const std::vector<std::shared_ptr<const FunctionNode>>& functions,
            const std::vector<std::shared_ptr<const FunctionTemplateNode>>& functionTemplates
        );
        virtual ~TemplatedImplNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const TemplatedImplNode> final;
        auto CreateBound() const -> Diagnosed<std::shared_ptr<const ImplBoundNode>> final;

        auto DefineAssociations() const -> Expected<void> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_SelfScope{};
        SymbolName m_TypeTemplateName{};
        std::vector<std::shared_ptr<const FunctionNode>> m_Functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> m_FunctionTemplates{};
    };
}
