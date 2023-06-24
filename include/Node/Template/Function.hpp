#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Template/Base.hpp"
#include "Node/Function.hpp"
#include "Node/TemplateParam/Impl.hpp"
#include "Node/TemplateParam/Normal.hpp"
#include "Scope.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Template
{
    class Function :
        public virtual Node::Template::IBase,
        public virtual Node::ICloneable<Node::Template::Function>
    {
    public:
        Function(
            const std::vector<std::shared_ptr<const Node::TemplateParam::Impl>>& t_implParams,
            const std::vector<std::shared_ptr<const Node::TemplateParam::Normal>>& t_params,
            const std::shared_ptr<const Node::Function>& t_ast
        );
        virtual ~Function() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Template::Function> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

        auto CollectImplParamNames() const -> std::vector<std::string> final;
        auto CollectParamNames()     const -> std::vector<std::string> final;

        auto GetAST() const -> const std::shared_ptr<const Node::Function>&;

    private:
        std::vector<std::shared_ptr<const Node::TemplateParam::Impl>>   m_ImplParams{};
        std::vector<std::shared_ptr<const Node::TemplateParam::Normal>> m_Params{};
        std::shared_ptr<const Node::Function> m_AST{};
    };
}
