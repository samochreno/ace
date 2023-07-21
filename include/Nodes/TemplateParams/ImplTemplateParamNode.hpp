#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"

namespace Ace
{
    class ImplTemplateParamNode :
        public virtual INode,
        public virtual ICloneableNode<ImplTemplateParamNode>,
        public virtual ISymbolCreatableNode
    {
    public:
        ImplTemplateParamNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            const Identifier& name
        );
        virtual ~ImplTemplateParamNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const ImplTemplateParamNode> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

        auto GetName() const -> const Identifier&;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Identifier m_Name{};
    };
}
