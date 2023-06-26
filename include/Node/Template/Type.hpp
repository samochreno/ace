#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Template/Base.hpp"
#include "Node/Type/Base.hpp"
#include "Node/TemplateParam/Impl.hpp"
#include "Node/TemplateParam/Normal.hpp"
#include "Scope.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace::Node::Template
{
    class Type :
        public virtual Node::Template::IBase,
        public virtual Node::ICloneable<Node::Template::Type>
    {
    public:
        Type(
            const std::vector<std::shared_ptr<const Node::TemplateParam::Normal>>& t_params,
            const std::shared_ptr<const Node::Type::IBase>& t_ast
        );
        virtual ~Type() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Template::Type> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> final;

        auto CollectImplParamNames() const -> std::vector<std::string> final;
        auto CollectParamNames()     const -> std::vector<std::string> final;

        auto GetAST() const -> const std::shared_ptr<const Node::Type::IBase>&;
        auto GetSelfScope() const -> std::shared_ptr<Scope>;

    private:
        std::vector<std::shared_ptr<const Node::TemplateParam::Normal>> m_Params{};
        std::shared_ptr<const Node::Type::IBase> m_AST{};
    };
}
