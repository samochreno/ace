#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Scope.hpp"

namespace Ace::Node::TemplateParameter
{
    class Impl :
        public virtual Node::IBase,
        public virtual Node::ICloneable<Node::TemplateParameter::Impl>,
        public virtual Node::ISymbolCreatable
    {
    public:
        Impl(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        ) : m_Scope{ t_scope },
            m_Name{ t_name }
        {
        }
        virtual ~Impl() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::TemplateParameter::Impl> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::ImplTemplateParameter; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

        auto GetName() const -> const std::string& { return m_Name; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
    };
}
