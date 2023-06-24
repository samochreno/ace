#include "Node/Var/Normal/Instance.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Attribute.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Var/Normal/Instance.hpp"
#include "Symbol/Var/Normal/Instance.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Var/Normal/Instance.hpp"

namespace Ace::Node::Var::Normal
{
    Instance::Instance(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const TypeName& t_typeName,
        const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
        const AccessModifier& t_accessModifier,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_TypeName{ t_typeName },
        m_Attributes{ t_attributes },
        m_AccessModifier{ t_accessModifier },
        m_Index{ t_index }
    {
    }

    auto Instance::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Instance::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto Instance::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Var::Normal::Instance>
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

        return std::make_shared<const Node::Var::Normal::Instance>(
            t_scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier,
            m_Index
        );
    }

    auto Instance::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Normal::Instance>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        auto* const selfSymbol =
            m_Scope->ExclusiveResolveSymbol<Symbol::Var::Normal::Instance>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Var::Normal::Instance>(
            selfSymbol,
            boundAttributes
        );
    }

    auto Instance::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Instance::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Instance::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::InstanceVar;
    }

    auto Instance::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Instance::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Var::Normal::Instance>(
                m_Scope,
                m_Name,
                m_AccessModifier,
                typeSymbol,
                m_Index
            )
        };
    }
}
