#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Nodes/TypedNode.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Nodes/Vars/Params/SelfParamVarNode.hpp"
#include "Nodes/Vars/Params/NormalParamVarNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    class FunctionNode :
        public virtual ITypedNode,
        public virtual ICloneableNode<FunctionNode>,
        public virtual IBindableNode<FunctionBoundNode>
    {
    public:
        FunctionNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& selfScope,
            const Ident& name,
            const TypeName& typeName,
            const std::vector<std::shared_ptr<const AttributeNode>>& attributes,
            const AccessModifier accessModifier,
            const std::optional<std::shared_ptr<const SelfParamVarNode>>& optSelf,
            const std::vector<std::shared_ptr<const NormalParamVarNode>>& params,
            const std::optional<std::shared_ptr<const BlockStmtNode>>& optBody
        );
        virtual ~FunctionNode() = default;

        auto GetSrcLocation() const -> const SrcLocation&;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const FunctionNode> final;
        auto CreateBound() const -> std::shared_ptr<const FunctionBoundNode> final;

        auto GetName() const -> const Ident& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;
        
        auto GetSelfScope() const -> std::shared_ptr<Scope>;
        auto GetAccessModifier() const -> AccessModifier;
        auto GetParams() const -> const std::vector<std::shared_ptr<const NormalParamVarNode>>&;

    protected:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_SelfScope{};
        Ident m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const AttributeNode>> m_Attributes{};
        SymbolCategory m_SymbolCategory{};
        AccessModifier m_AccessModifier{};
        std::optional<std::shared_ptr<const SelfParamVarNode>> m_OptSelf{};
        std::vector<std::shared_ptr<const NormalParamVarNode>> m_Params{};
        std::optional<std::shared_ptr<const BlockStmtNode>> m_OptBody{};
    };
}
