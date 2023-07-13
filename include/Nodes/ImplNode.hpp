#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ImplNode :
        public virtual INode,
        public virtual ICloneableNode<ImplNode>,
        public virtual IBindableNode<ImplBoundNode>
    {
    public:
        ImplNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_selfScope,
            const SymbolName& t_typeName,
            const std::vector<std::shared_ptr<const FunctionNode>>& t_functions,
            const std::vector<std::shared_ptr<const FunctionTemplateNode>>& t_functionTemplates
        );
        virtual ~ImplNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const ImplNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const ImplBoundNode>> final;

        auto DefineAssociations() const -> Expected<void>;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_SelfScope{};
        SymbolName m_TypeName{};
        std::vector<std::shared_ptr<const FunctionNode>> m_Functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> m_FunctionTemplates{};
    };
}
