#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Type/Base.hpp"
#include "Node/Attribute.hpp"
#include "Node/Variable/Normal/Instance.hpp"
#include "BoundNode/Type/Struct.hpp"
#include "Diagnostics.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Type
{
    class Struct :
        public virtual Node::Type::IBase,
        public virtual Node::ICloneable<Node::Type::Struct>,
        public virtual Node::IBindable<BoundNode::Type::Struct>
    {
    public:
        Struct(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
            const AccessModifier& t_accessModifier,
            const std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>>& t_variables
        );
        virtual ~Struct() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Type::Struct> final;
        auto CloneInScopeType(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Type::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Type::Struct>> final;
        auto CreateBoundType() const -> Expected<std::shared_ptr<const BoundNode::Type::IBase>> final;

        auto GetName() const -> const std::string& final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCreationSuborder() const -> size_t final;
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;
        
    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        std::vector<std::shared_ptr<const Node::Attribute>> m_Attributes{};
        AccessModifier m_AccessModifier{};
        std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>> m_Variables{};
    };
}
