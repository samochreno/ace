#include "Node/Var/Normal/Static.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Attribute.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Var/Normal/Static.hpp"
#include "Symbol/Var/Normal/Static.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Var/Normal/Static.hpp"

namespace Ace::Node::Var::Normal
{
    Static::Static(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const TypeName& t_typeName,
        const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
        const AccessModifier& t_accessModifier
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_TypeName{ t_typeName },
        m_Attributes{ t_attributes },
        m_AccessModifier{ t_accessModifier }
    {
    }

    auto Static::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Static::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto Static::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Var::Normal::Static>
    {
        std::vector<std::shared_ptr<const Node::Attribute>> clonedAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(clonedAttributes),
            [&](const std::shared_ptr<const Node::Attribute>& t_attribute)
            {
                return t_attribute->CloneInScope(t_scope);
            }
        );

        return std::make_shared<const Node::Var::Normal::Static>(
            t_scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier
        );
    }

    auto Static::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Normal::Static>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        auto* const selfSymbol =
            m_Scope->ExclusiveResolveSymbol<Symbol::Var::Normal::Static>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Var::Normal::Static>(
            selfSymbol,
            boundAttributes
        );
    }

    auto Static::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Static::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Static::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::StaticVar;
    }

    auto Static::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Static::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Var::Normal::Static>(
                m_Scope,
                m_Name,
                m_AccessModifier,
                typeSymbol
                )
        };
    }
}
