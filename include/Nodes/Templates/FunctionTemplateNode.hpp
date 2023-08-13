#pragma once

#include <memory>
#include <vector>

#include "Nodes/Templates/TemplateNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/TemplateParams/ImplTemplateParamNode.hpp"
#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Ident.hpp"

namespace Ace
{
    class FunctionTemplateNode :
        public std::enable_shared_from_this<FunctionTemplateNode>,
        public virtual ITemplateNode,
        public virtual ICloneableInScopeNode<FunctionTemplateNode>
    {
    public:
        FunctionTemplateNode(
            const SrcLocation& srcLocation,
            const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& implParams,
            const std::vector<std::shared_ptr<const NormalTemplateParamNode>>& params,
            const std::shared_ptr<const FunctionNode>& ast
        );
        virtual ~FunctionTemplateNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const FunctionTemplateNode> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

        auto GetAST() const -> std::shared_ptr<const ITemplatableNode> final;

        auto CollectImplParamNames() const -> std::vector<Ident> final;
        auto CollectParamNames()     const -> std::vector<Ident> final;

        auto GetConcreteAST() const -> const std::shared_ptr<const FunctionNode>&;

    private:
        SrcLocation m_SrcLocation{};
        std::vector<std::shared_ptr<const ImplTemplateParamNode>>   m_ImplParams{};
        std::vector<std::shared_ptr<const NormalTemplateParamNode>> m_Params{};
        std::shared_ptr<const FunctionNode> m_AST{};
    };
}
