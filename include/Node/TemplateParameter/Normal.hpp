
#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Scope.hpp"

namespace Ace::Node::TemplateParameter
{
    class Normal :
        public virtual Node::IBase,
        public virtual Node::ICloneable<Node::TemplateParameter::Normal>,
        public virtual Node::ISymbolCreatable
    {
    public:
        Normal(
            Scope* const t_scope,
            const std::string& t_name
        ) : m_Scope{ t_scope },
            m_Name{ t_name }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::TemplateParameter::Normal> final;

        auto GetSymbolScope() const -> Scope* final { return m_Scope; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::TemplateParameter; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

        auto GetName() const -> const std::string& { return m_Name; }

    private:
        Scope* m_Scope{};
        std::string m_Name{};
    };
}

