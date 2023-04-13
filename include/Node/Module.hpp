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
#include "Node/Variable/Normal/Static.hpp"
#include "BoundNode/Module.hpp"
#include "SymbolKind.hpp"
#include "Symbol/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Error.hpp"

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
            Scope* const t_scope,
            Scope* const t_selfScope,
            const std::vector<std::string>& t_name,
            const AccessModifier& t_accessModifier,
            const std::vector<std::shared_ptr<const Node::Module>>& t_modules,
            const std::vector<std::shared_ptr<const Node::Type::IBase>>& t_types,
            const std::vector<std::shared_ptr<const Node::Template::Type>>& t_typeTemplates,
            const std::vector<std::shared_ptr<const Node::Impl>>& t_impls,
            const std::vector<std::shared_ptr<const Node::TemplatedImpl>>& t_templatedImpls,
            const std::vector<std::shared_ptr<const Node::Function>>& t_functions,
            const std::vector<std::shared_ptr<const Node::Template::Function>>& t_functionTemplates,
            const std::vector<std::shared_ptr<const Node::Variable::Normal::Static>>& t_variables
        ) : m_Scope{ t_scope },
            m_SelfScope{ t_selfScope },
            m_Name{ t_name },
            m_AccessModifier{ t_accessModifier },
            m_Modules{ t_modules },
            m_Types{ t_types },
            m_TypeTemplates{ t_typeTemplates },
            m_Impls{ t_impls },
            m_TemplatedImpls{ t_templatedImpls },
            m_Functions{ t_functions },
            m_FunctionTemplates{ t_functionTemplates },
            m_Variables{ t_variables }
        {
        }
        virtual ~Module() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Module> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Module>> final;

        auto GetSymbolScope() const -> Scope* final;
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::Module; }
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;
        auto ContinueCreatingSymbol(Symbol::IBase* const t_symbol) const -> Expected<void> final;
        auto GetName() const -> const std::string& final;

    private:
        Scope* m_Scope{};
        Scope* m_SelfScope{};
        std::vector<std::string> m_Name{};
        AccessModifier m_AccessModifier{};
        std::vector<std::shared_ptr<const Node::Module>> m_Modules{};
        std::vector<std::shared_ptr<const Node::Type::IBase>> m_Types{};
        std::vector<std::shared_ptr<const Node::Template::Type>> m_TypeTemplates{};
        std::vector<std::shared_ptr<const Node::Impl>> m_Impls{};
        std::vector<std::shared_ptr<const Node::TemplatedImpl>> m_TemplatedImpls{};
        std::vector<std::shared_ptr<const Node::Function>> m_Functions{};
        std::vector<std::shared_ptr<const Node::Template::Function>> m_FunctionTemplates{};
        std::vector<std::shared_ptr<const Node::Variable::Normal::Static>> m_Variables{};
    };
}
