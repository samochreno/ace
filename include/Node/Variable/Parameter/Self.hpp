#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Base.hpp"
#include "Node/Typed.hpp"
#include "Node/Attribute.hpp"
#include "BoundNode/Variable/Parameter/Self.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "SpecialIdentifier.hpp"
#include "Error.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Variable::Parameter
{
    class Self :
        public virtual Node::IBase,
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Variable::Parameter::Self>,
        public virtual Node::IBindable<BoundNode::Variable::Parameter::Self>
    {
    public:
        Self(
            Scope* const t_scope,
            const Name::Symbol::Full& t_typeName
        ) : m_Scope{ t_scope },
            m_Name{ SpecialIdentifier::Self },
            m_TypeName{ t_typeName, std::vector{ Name::TypeModifier::Reference } }
        {
        }
        virtual ~Self() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Variable::Parameter::Self> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Variable::Parameter::Self>> final;

        auto GetName() const -> const std::string& final { return m_Name; }

        auto GetSymbolScope() const -> Scope* final { return m_Scope; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::Function; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        Name::Type m_TypeName{};
    };
}
