#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Templates/TemplateNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/TemplateParams/ImplTemplateParamNode.hpp"
#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"
#include "Scope.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class FunctionTemplateNode :
        public virtual ITemplateNode,
        public virtual ICloneableNode<FunctionTemplateNode>
    {
    public:
        FunctionTemplateNode(
            const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& t_implParams,
            const std::vector<std::shared_ptr<const NormalTemplateParamNode>>& t_params,
            const std::shared_ptr<const FunctionNode>& t_ast
        );
        virtual ~FunctionTemplateNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const FunctionTemplateNode> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

        auto CollectImplParamNames() const -> std::vector<std::string> final;
        auto CollectParamNames()     const -> std::vector<std::string> final;

        auto GetAST() const -> const std::shared_ptr<const FunctionNode>&;

    private:
        std::vector<std::shared_ptr<const ImplTemplateParamNode>>   m_ImplParams{};
        std::vector<std::shared_ptr<const NormalTemplateParamNode>> m_Params{};
        std::shared_ptr<const FunctionNode> m_AST{};
    };
}
