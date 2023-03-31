#include "Node/Variable/Parameter/Normal.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Attribute.hpp"
#include "Error.hpp"
#include "BoundNode/Variable/Parameter/Normal.hpp"
#include "Symbol/Variable/Parameter/Normal.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Variable::Parameter
{
    auto Normal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto Normal::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Variable::Parameter::Normal>
    {
        std::vector<std::shared_ptr<const Node::Attribute>> clonedAttributes{};
        std::transform(begin(m_Attributes), end(m_Attributes), back_inserter(clonedAttributes), [&]
        (const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CloneInScope(t_scope);
        });

        return std::make_unique<const Node::Variable::Parameter::Normal>(
            t_scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_Index
            );
    }

    auto Normal::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Variable::Parameter::Normal>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes, []
        (const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<Symbol::Variable::Parameter::Normal>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Variable::Parameter::Normal>(
            selfSymbol,
            boundAttributes
            );
    }

    auto Normal::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::IBase>(m_TypeName.ToSymbolName()));

        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Variable::Parameter::Normal>(
                m_Scope,
                m_Name,
                typeSymbol,
                m_Index
                )
        };
    }
}
