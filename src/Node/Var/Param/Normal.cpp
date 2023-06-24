#include "Node/Var/Param/Normal.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Attribute.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Var/Param/Normal.hpp"
#include "Symbol/Var/Param/Normal.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Var::Param
{
    Normal::Normal(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const TypeName& t_typeName,
        const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_TypeName{ t_typeName },
        m_Attributes{ t_attributes },
        m_Index{ t_index }
    {
    }

    auto Normal::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto Normal::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Var::Param::Normal>
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

        return std::make_shared<const Node::Var::Param::Normal>(
            t_scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_Index
        );
    }

    auto Normal::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Param::Normal>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        auto* const selfSymbol =
            m_Scope->ExclusiveResolveSymbol<Symbol::Var::Param::Normal>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Var::Param::Normal>(
            selfSymbol,
            boundAttributes
        );
    }

    auto Normal::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Normal::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ParamVar;
    }

    auto Normal::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Normal::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Var::Param::Normal>(
                m_Scope,
                m_Name,
                typeSymbol,
                m_Index
            )
        };
    }
}
