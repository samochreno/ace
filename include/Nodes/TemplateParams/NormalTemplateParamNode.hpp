
#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Node.hpp"
#include "Scope.hpp"

namespace Ace
{
    class NormalTemplateParamNode :
        public virtual INode,
        public virtual ICloneableNode<NormalTemplateParamNode>,
        public virtual ISymbolCreatableNode
    {
    public:
        NormalTemplateParamNode(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        );
        virtual ~NormalTemplateParamNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const NormalTemplateParamNode> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

        auto GetName() const -> const std::string&;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
    };
}
