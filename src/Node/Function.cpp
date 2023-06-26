#include "Node/Function.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Node/Attribute.hpp"
#include "Node/Var/Param/Self.hpp"
#include "Node/Var/Param/Normal.hpp"
#include "Node/Stmt/Block.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Function.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace::Node
{
    Function::Function(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::string& t_name,
        const TypeName& t_typeName,
        const std::vector<std::shared_ptr<const Node::Attribute>>& t_attributes,
        const AccessModifier& t_accessModifier,
        const std::optional<std::shared_ptr<const Node::Var::Param::Self>>& t_optSelf,
        const std::vector<std::shared_ptr<const Node::Var::Param::Normal>>& t_params,
        const std::optional<std::shared_ptr<const Node::Stmt::Block>>& t_optBody
    ) : m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_TypeName{ t_typeName },
        m_Attributes{ t_attributes },
        m_AccessModifier{ t_accessModifier },
        m_OptSelf{ t_optSelf },
        m_Params{ t_params },
        m_OptBody{ t_optBody }
    {
    }

    auto Function::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Function::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Attributes);

        if (m_OptSelf.has_value())
        {
            AddChildren(children, m_OptSelf.value());
        }

        AddChildren(children, m_Params);

        if (m_OptBody.has_value())
        {
            AddChildren(children, m_OptBody.value());
        }

        return children;
    }

    auto Function::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Function>
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

        const auto clonedOptSelf = [&]() -> std::optional<std::shared_ptr<const Node::Var::Param::Self>>
        {
            if (!m_OptSelf.has_value())
                return std::nullopt;

            return m_OptSelf.value()->CloneInScope(selfScope);
        }();

        std::vector<std::shared_ptr<const Node::Var::Param::Normal>> clonedParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(clonedParams),
            [&](const std::shared_ptr<const Node::Var::Param::Normal>& t_param)
            {
                return t_param->CloneInScope(selfScope);
            }
        );

        const auto clonedOptBody = [&]() -> std::optional<std::shared_ptr<const Node::Stmt::Block>>
        {
            if (!m_OptBody.has_value())
            {
                return std::nullopt;
            }

            return m_OptBody.value()->CloneInScope(selfScope);
        }();

        return std::make_shared<const Node::Function>(
            selfScope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier,
            clonedOptSelf,
            clonedParams,
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
        [](const std::shared_ptr<const Node::Var::Param::Self>& t_param)
        {
            return t_param->CreateBound();
        }));

        ACE_TRY(boundParams, TransformExpectedVector(m_Params,
        [](const std::shared_ptr<const Node::Var::Param::Normal>& t_param)
        {
            return t_param->CreateBound();
        }));

        if (m_Name == "implicit_conversion_with_operator")
        {
            [](){}();
        }

        ACE_TRY(boundOptBody, TransformExpectedOptional(m_OptBody,
        [](const std::shared_ptr<const Node::Stmt::Block>& t_body)
        {
            return t_body->CreateBound();
        }));

        ACE_TRY(typeSymbol, m_SelfScope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        auto* const selfSymbol = GetScope()->ExclusiveResolveSymbol<FunctionSymbol>(
            m_Name,
            m_SelfScope->CollectImplTemplateArgs(),
            m_SelfScope->CollectTemplateArgs()
        ).Unwrap();

        return std::make_shared<const BoundNode::Function>(
            selfSymbol,
            boundAttributes,
            boundOptSelf,
            boundParams,
            boundOptBody
        );
    }

    auto Function::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Function::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto Function::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Function;
    }

    auto Function::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Function::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        ACE_TRY(typeSymbol, m_SelfScope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<ISymbol>
        {
            std::make_unique<FunctionSymbol>(
                m_SelfScope,
                m_Name,
                m_OptSelf.has_value() ?
                    SymbolCategory::Instance :
                    SymbolCategory::Static,
                m_AccessModifier,
                typeSymbol
            )
        };
    }
    
    auto Function::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto Function::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto Function::GetParams() const -> const std::vector<std::shared_ptr<const Node::Var::Param::Normal>>&
    {
        return m_Params;
    }
}
