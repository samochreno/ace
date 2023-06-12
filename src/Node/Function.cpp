#include "Node/Function.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include <algorithm>
#include <iterator>

#include "Scope.hpp"
#include "Node/Attribute.hpp"
#include "Node/Variable/Parameter/Self.hpp"
#include "Node/Variable/Parameter/Normal.hpp"
#include "Node/Statement/Block.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Function.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node
{
    auto Function::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Attributes);

        if (m_OptSelf.has_value())
        {
            AddChildren(children, m_OptSelf.value());
        }

        AddChildren(children, m_Parameters);

        if (m_OptBody.has_value())
        {
            AddChildren(children, m_OptBody.value());
        }

        return children;
    }

    auto Function::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Function>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const Node::Attribute>> clonedAttributes{};
        std::transform(begin(m_Attributes), end(m_Attributes), back_inserter(clonedAttributes),
        [&](const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CloneInScope(t_scope);
        });

        const auto clonedOptSelf = [&]() -> std::optional<std::shared_ptr<const Node::Variable::Parameter::Self>>
        {
            if (!m_OptSelf.has_value())
                return std::nullopt;

            return m_OptSelf.value()->CloneInScope(selfScope);
        }();

        std::vector<std::shared_ptr<const Node::Variable::Parameter::Normal>> clonedParameters{};
        std::transform(begin(m_Parameters), end(m_Parameters), back_inserter(clonedParameters),
        [&](const std::shared_ptr<const Node::Variable::Parameter::Normal>& t_parameter)
        {
            return t_parameter->CloneInScope(selfScope);
        });

        const auto clonedOptBody = [&]() -> std::optional<std::shared_ptr<const Node::Statement::Block>>
        {
            if (!m_OptBody.has_value())
                return std::nullopt;

            return m_OptBody.value()->CloneInScope(selfScope);
        }();

        return std::make_shared<const Node::Function>(
            selfScope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier,
            clonedOptSelf,
            clonedParameters,
            clonedOptBody
        );
    }

    auto Function::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Function>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const Node::Attribute>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        ACE_TRY(boundOptSelf, TransformExpectedOptional(m_OptSelf,
        [](const std::shared_ptr<const Node::Variable::Parameter::Self>& t_parameter)
        {
            return t_parameter->CreateBound();
        }));

        ACE_TRY(boundParameters, TransformExpectedVector(m_Parameters,
        [](const std::shared_ptr<const Node::Variable::Parameter::Normal>& t_parameter)
        {
            return t_parameter->CreateBound();
        }));

        if (m_Name == "implicit_conversion_with_operator")
        {
            [](){}();
        }

        ACE_TRY(boundOptBody, TransformExpectedOptional(m_OptBody,
        [](const std::shared_ptr<const Node::Statement::Block>& t_body)
        {
            return t_body->CreateBound();
        }));

        ACE_TRY(typeSymbol, m_SelfScope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        auto* const selfSymbol = GetScope()->ExclusiveResolveSymbol<Symbol::Function>(
            m_Name,
            m_SelfScope->CollectImplTemplateArguments(),
            m_SelfScope->CollectTemplateArguments()
        ).Unwrap();

        return std::make_shared<const BoundNode::Function>(
            selfSymbol,
            boundAttributes,
            boundOptSelf,
            boundParameters,
            boundOptBody
        );
    }

    auto Function::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(typeSymbol, m_SelfScope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Function>(
                m_SelfScope,
                m_Name,
                m_OptSelf.has_value() ? SymbolCategory::Instance : SymbolCategory::Static,
                m_AccessModifier,
                typeSymbol
            )
        };
    }
}
