#include "Node/Type/Struct.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Type/Struct.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Type/Struct.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Type
{
    Struct::Struct(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::string& t_name,
        const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
        const AccessModifier& t_accessModifier,
        const std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>>& t_variables
    ) : m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_Attributes{ t_attributes },
        m_AccessModifier{ t_accessModifier },
        m_Variables{ t_variables }
    {
    }

    auto Struct::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Struct::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto Struct::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Attributes);
        AddChildren(children, m_Variables);

        return children;
    }

    auto Struct::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Type::Struct>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

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

        std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>> clonedVariables{};
        std::transform(
            begin(m_Variables),
            end  (m_Variables),
            back_inserter(clonedVariables),
            [&](const std::shared_ptr<const Node::Variable::Normal::Instance>& t_variable)
            {
                return t_variable->CloneInScope(selfScope);
            }
        );

        return std::make_shared<const Node::Type::Struct>(
            selfScope,
            m_Name,
            clonedAttributes,
            m_AccessModifier,
            clonedVariables
        );
    }

    auto Struct::CloneInScopeType(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Type::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Struct::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Type::Struct>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        ACE_TRY(boundVariables, TransformExpectedVector(m_Variables,
        [](const std::shared_ptr<const Node::Variable::Normal::Instance>& t_variable)
        {
            return t_variable->CreateBound();
        }));

        auto* const selfSymbol = GetScope()->ExclusiveResolveSymbol<Symbol::Type::Struct>(
            m_Name,
            m_SelfScope->CollectImplTemplateArguments(),
            m_SelfScope->CollectTemplateArguments()
        ).Unwrap();

        return std::make_shared<const BoundNode::Type::Struct>(
            selfSymbol,
            boundAttributes,
            boundVariables
        );
    }

    auto Struct::CreateBoundType() const -> Expected<std::shared_ptr<const BoundNode::Type::IBase>>
    {
        return CreateBound();
    }

    auto Struct::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Struct::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto Struct::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto Struct::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Struct;
    }

    auto Struct::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Struct::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Type::Struct>(
                m_SelfScope,
                m_Name,
                m_AccessModifier
            )
        };
    }
}
