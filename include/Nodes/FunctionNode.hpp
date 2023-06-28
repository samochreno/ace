#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Nodes/TypedNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Nodes/Vars/Params/SelfParamVarNode.hpp"
#include "Nodes/Vars/Params/NormalParamVarNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNode/Function.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Diagnostics.hpp"
#include "Scope.hpp"
#include "Symbols/Symbol.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace
{
    class FunctionNode :
        public virtual ITypedNode,
        public virtual ICloneableNode<FunctionNode>,
        public virtual IBindableNode<BoundNode::Function>
    {
    public:
        FunctionNode(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const TypeName& t_typeName,
            const std::vector<std::shared_ptr<const AttributeNode>>& t_attributes,
            const AccessModifier& t_accessModifier,
            const std::optional<std::shared_ptr<const SelfParamVarNode>>& t_optSelf,
            const std::vector<std::shared_ptr<const NormalParamVarNode>>& t_params,
            const std::optional<std::shared_ptr<const BlockStmtNode>>& t_optBody
        );
        virtual ~FunctionNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const FunctionNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Function>> final;

        auto GetName() const -> const std::string& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;
        
        auto GetSelfScope() const -> std::shared_ptr<Scope>;
        auto GetAccessModifier() const -> AccessModifier;
        auto GetParams() const -> const std::vector<std::shared_ptr<const NormalParamVarNode>>&;

    protected:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const AttributeNode>> m_Attributes{};
        SymbolCategory m_SymbolCategory{};
        AccessModifier m_AccessModifier{};
        std::optional<std::shared_ptr<const SelfParamVarNode>> m_OptSelf{};
        std::vector<std::shared_ptr<const NormalParamVarNode>> m_Params{};
        std::optional<std::shared_ptr<const BlockStmtNode>> m_OptBody{};
    };
}
