
#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"

namespace Ace
{
    class NormalTemplateParamNode :
        public virtual INode,
        public virtual ICloneableNode<NormalTemplateParamNode>,
        public virtual ISymbolCreatableNode
    {
    public:
        NormalTemplateParamNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const Ident& name
        );
        virtual ~NormalTemplateParamNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const NormalTemplateParamNode> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

        auto GetName() const -> const Ident&;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
    };
}
