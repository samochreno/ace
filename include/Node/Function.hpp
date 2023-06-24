#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Node/Typed.hpp"
#include "Node/Attribute.hpp"
#include "Node/Variable/Parameter/Self.hpp"
#include "Node/Variable/Parameter/Normal.hpp"
#include "Node/Statement/Block.hpp"
#include "BoundNode/Function.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Diagnostics.hpp"
#include "Scope.hpp"
#include "Symbol/Base.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Node
{
    class Function :
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Function>,
        public virtual Node::IBindable<BoundNode::Function>
    {
    public:
        Function(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const TypeName& t_typeName,
            const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
            const AccessModifier& t_accessModifier,
            const std::optional<std::shared_ptr<const Node::Variable::Parameter::Self>>& t_optSelf,
            const std::vector<std::shared_ptr<const Node::Variable::Parameter::Normal>>& t_parameters,
            const std::optional<std::shared_ptr<const Node::Statement::Block>>& t_optBody
        );
        virtual ~Function() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Function> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Function>> final;

        auto GetName() const -> const std::string& final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;
        
        auto GetSelfScope() const -> std::shared_ptr<Scope>;
        auto GetAccessModifier() const -> AccessModifier;
        auto GetParameters() const -> const std::vector<std::shared_ptr<const Node::Variable::Parameter::Normal>>&;

    protected:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const Node::Attribute>> m_Attributes{};
        SymbolCategory m_SymbolCategory{};
        AccessModifier m_AccessModifier{};
        std::optional<std::shared_ptr<const Node::Variable::Parameter::Self>> m_OptSelf{};
        std::vector<std::shared_ptr<const Node::Variable::Parameter::Normal>> m_Parameters{};
        std::optional<std::shared_ptr<const Node::Statement::Block>> m_OptBody{};
    };
}
