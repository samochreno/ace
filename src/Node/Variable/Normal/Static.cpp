#include "Node/Variable/Normal/Static.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Attribute.hpp"
#include "Error.hpp"
#include "BoundNode/Variable/Normal/Static.hpp"
#include "Symbol/Variable/Normal/Static.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Variable/Normal/Static.hpp"

namespace Ace::Node::Variable::Normal
{
    auto Static::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto Static::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Variable::Normal::Static>
    {
        std::vector<std::shared_ptr<const Node::Attribute>> clonedAttributes{};
        std::transform(begin(m_Attributes), end(m_Attributes), back_inserter(clonedAttributes),
        [&](const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CloneInScope(t_scope);
        });

        return std::make_shared<const Node::Variable::Normal::Static>(
            t_scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier
        );
    }

    auto Static::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Variable::Normal::Static>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<Symbol::Variable::Normal::Static>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Variable::Normal::Static>(
            selfSymbol,
            boundAttributes
            );
    }

    auto Static::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Variable::Normal::Static>(
                m_Scope,
                m_Name,
                m_AccessModifier,
                typeSymbol
                )
        };
    }
}
