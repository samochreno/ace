#pragma once

#include <memory>
#include <vector>

#include "Node/Base.hpp"
#include "Node/Function.hpp"
#include "Node/Template/Function.hpp"
#include "BoundNode/Impl.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Symbol/Kind.hpp"
#include "Symbol/Base.hpp"
#include "Error.hpp"

namespace Ace::Node
{
    class TemplatedImpl :
        public virtual Node::IBase,
        public virtual Node::ICloneable<Node::TemplatedImpl>,
        public virtual Node::IBindable<BoundNode::Impl>,
        public virtual Node::ISymbolCreatable
    {
    public:
        TemplatedImpl(
            Scope* const t_selfScope,
            const Name::Symbol::Full& t_typeTemplateName,
            const std::vector<std::shared_ptr<const Node::Function>>& t_functions,
            const std::vector<std::shared_ptr<const Node::Template::Function>>& t_functionTemplates
        ) : m_SelfScope{ t_selfScope },
            m_TypeTemplateName{ t_typeTemplateName },
            m_Functions{ t_functions },
            m_FunctionTemplates{ t_functionTemplates }
        {
        }
        virtual ~TemplatedImpl() = default;

        auto GetScope() const -> Scope* final { return m_SelfScope->GetParent(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::TemplatedImpl> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Impl>> final;

        auto GetSymbolScope() const -> Scope* final { return GetScope(); }
        auto GetSymbolKind() const -> Symbol::Kind final { return Symbol::Kind::TemplatedImpl; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

        auto DefineAssociations() const -> Expected<void>;

    private:
        Scope* m_SelfScope{};
        Name::Symbol::Full m_TypeTemplateName{};
        std::vector<std::shared_ptr<const Node::Function>> m_Functions{};
        std::vector<std::shared_ptr<const Node::Template::Function>> m_FunctionTemplates{};
    };
}
