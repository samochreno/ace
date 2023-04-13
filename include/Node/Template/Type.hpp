#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Template/Base.hpp"
#include "Node/Type/Base.hpp"
#include "Scope.hpp"
#include "SymbolKind.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Template
{
    class Type :
        public virtual Node::Template::IBase,
        public virtual Node::ICloneable<Node::Template::Type>
    {
    public:
        Type(
            const std::vector<std::string>& t_parameters,
            const std::shared_ptr<const Node::Type::IBase>& t_ast
        ) : m_Parameters{ t_parameters },
            m_AST{ t_ast }
        {
        }
        virtual ~Type() = default;

        auto GetScope() const -> Scope* final { return m_AST->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Template::Type> final;

        auto GetSymbolScope() const -> Scope* final { return GetScope(); }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::TypeTemplate; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

        auto GetImplParameters() const -> const std::vector<std::string>& final;
        auto GetParameters() const -> const std::vector<std::string>& final { return m_Parameters; }

        auto GetAST() const -> const std::shared_ptr<const Node::Type::IBase>& { return m_AST; }
        auto GetSelfScope() const -> Scope* { return m_AST->GetSelfScope(); }

    private:
        std::vector<std::string> m_Parameters{};
        std::shared_ptr<const Node::Type::IBase> m_AST{};
    };
}
