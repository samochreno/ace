#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Node/Type/Base.hpp"
#include "Node/Template/Type.hpp"
#include "Node/Impl.hpp"
#include "Node/TemplatedImpl.hpp"
#include "Node/Function.hpp"
#include "Node/Template/Function.hpp"
#include "Node/Var/Normal/Static.hpp"
#include "BoundNode/Module.hpp"
#include "Symbols/Symbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node
{
    class Module :
        public virtual Node::IBase,
        public virtual Node::ICloneable<Node::Module>,
        public virtual Node::IBindable<BoundNode::Module>,
        public virtual Node::IPartiallySymbolCreatable
    {
    public:
        Module(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<Scope>& t_selfScope,
            const std::vector<std::string>& t_name,
            const AccessModifier& t_accessModifier,
            const std::vector<std::shared_ptr<const Node::Module>>& t_modules,
            const std::vector<std::shared_ptr<const Node::Type::IBase>>& t_types,
            const std::vector<std::shared_ptr<const Node::Template::Type>>& t_typeTemplates,
            const std::vector<std::shared_ptr<const Node::Impl>>& t_impls,
            const std::vector<std::shared_ptr<const Node::TemplatedImpl>>& t_templatedImpls,
            const std::vector<std::shared_ptr<const Node::Function>>& t_functions,
            const std::vector<std::shared_ptr<const Node::Template::Function>>& t_functionTemplates,
            const std::vector<std::shared_ptr<const Node::Var::Normal::Static>>& t_variables
        );
        virtual ~Module() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Module> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Module>> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;
        auto ContinueCreatingSymbol(
            ISymbol* const t_symbol
        ) const -> Expected<void> final;
        auto GetName() const -> const std::string& final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<Scope> m_SelfScope{};
        std::vector<std::string> m_Name{};
        AccessModifier m_AccessModifier{};
        std::vector<std::shared_ptr<const Node::Module>> m_Modules{};
        std::vector<std::shared_ptr<const Node::Type::IBase>> m_Types{};
        std::vector<std::shared_ptr<const Node::Template::Type>> m_TypeTemplates{};
        std::vector<std::shared_ptr<const Node::Impl>> m_Impls{};
        std::vector<std::shared_ptr<const Node::TemplatedImpl>> m_TemplatedImpls{};
        std::vector<std::shared_ptr<const Node::Function>> m_Functions{};
        std::vector<std::shared_ptr<const Node::Template::Function>> m_FunctionTemplates{};
        std::vector<std::shared_ptr<const Node::Var::Normal::Static>> m_Vars{};
    };
}
