#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Nodes/Node.hpp"
#include "Nodes/Types/TypeNode.hpp"
#include "Nodes/Templates/TypeTemplateNode.hpp"
#include "Nodes/ImplNode.hpp"
#include "Nodes/TemplatedImplNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "Nodes/Vars/StaticVarNode.hpp"
#include "BoundNodes/ModuleBoundNode.hpp"
#include "Symbols/Symbol.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ModuleNode :
        public virtual INode,
        public virtual ICloneableNode<ModuleNode>,
        public virtual IBindableNode<ModuleBoundNode>,
        public virtual IPartiallySymbolCreatableNode
    {
    public:
        ModuleNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<Scope>& t_selfScope,
            const std::vector<Identifier>& t_name,
            const AccessModifier t_accessModifier,
            const std::vector<std::shared_ptr<const ModuleNode>>& t_modules,
            const std::vector<std::shared_ptr<const ITypeNode>>& t_types,
            const std::vector<std::shared_ptr<const TypeTemplateNode>>& t_typeTemplates,
            const std::vector<std::shared_ptr<const ImplNode>>& t_impls,
            const std::vector<std::shared_ptr<const TemplatedImplNode>>& t_templatedImpls,
            const std::vector<std::shared_ptr<const FunctionNode>>& t_functions,
            const std::vector<std::shared_ptr<const FunctionTemplateNode>>& t_functionTemplates,
            const std::vector<std::shared_ptr<const StaticVarNode>>& t_vars
        );
        virtual ~ModuleNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const ModuleNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const ModuleBoundNode>> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;
        auto ContinueCreatingSymbol(
            ISymbol* const t_symbol
        ) const -> Expected<void> final;
        auto GetName() const -> const Identifier& final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<Scope> m_SelfScope{};
        std::vector<Identifier> m_Name{};
        AccessModifier m_AccessModifier{};
        std::vector<std::shared_ptr<const ModuleNode>> m_Modules{};
        std::vector<std::shared_ptr<const ITypeNode>> m_Types{};
        std::vector<std::shared_ptr<const TypeTemplateNode>> m_TypeTemplates{};
        std::vector<std::shared_ptr<const ImplNode>> m_Impls{};
        std::vector<std::shared_ptr<const TemplatedImplNode>> m_TemplatedImpls{};
        std::vector<std::shared_ptr<const FunctionNode>> m_Functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> m_FunctionTemplates{};
        std::vector<std::shared_ptr<const StaticVarNode>> m_Vars{};
    };
}
