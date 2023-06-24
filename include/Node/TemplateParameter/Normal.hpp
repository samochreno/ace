
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
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        );
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::TemplateParameter::Normal> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

        auto GetName() const -> const std::string&;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
    };
}
