#include "Nodes/FunctionNode.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Nodes/Vars/Params/SelfParamVarNode.hpp"
#include "Nodes/Vars/Params/NormalParamVarNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    FunctionNode::FunctionNode(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::string& t_name,
        const TypeName& t_typeName,
        const std::vector<std::shared_ptr<const AttributeNode>>& t_attributes,
        const AccessModifier t_accessModifier,
        const std::optional<std::shared_ptr<const SelfParamVarNode>>& t_optSelf,
        const std::vector<std::shared_ptr<const NormalParamVarNode>>& t_params,
        const std::optional<std::shared_ptr<const BlockStmtNode>>& t_optBody
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

    auto FunctionNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto FunctionNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

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

    auto FunctionNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const FunctionNode>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const AttributeNode>> clonedAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(clonedAttributes),
            [&](const std::shared_ptr<const AttributeNode>& t_attribute)
            {
                return t_attribute->CloneInScope(t_scope);
            }
        );

        const auto clonedOptSelf = [&]() -> std::optional<std::shared_ptr<const SelfParamVarNode>>
        {
            if (!m_OptSelf.has_value())
                return std::nullopt;

            return m_OptSelf.value()->CloneInScope(selfScope);
        }();

        std::vector<std::shared_ptr<const NormalParamVarNode>> clonedParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(clonedParams),
            [&](const std::shared_ptr<const NormalParamVarNode>& t_param)
            {
                return t_param->CloneInScope(selfScope);
            }
        );

        const auto clonedOptBody = [&]() -> std::optional<std::shared_ptr<const BlockStmtNode>>
        {
            if (!m_OptBody.has_value())
            {
                return std::nullopt;
            }

            return m_OptBody.value()->CloneInScope(selfScope);
        }();

        return std::make_shared<const FunctionNode>(
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

    auto FunctionNode::CreateBound() const -> Expected<std::shared_ptr<const FunctionBoundNode>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeNode>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        ACE_TRY(boundOptSelf, TransformExpectedOptional(m_OptSelf,
        [](const std::shared_ptr<const SelfParamVarNode>& t_param)
        {
            return t_param->CreateBound();
        }));

        ACE_TRY(boundParams, TransformExpectedVector(m_Params,
        [](const std::shared_ptr<const NormalParamVarNode>& t_param)
        {
            return t_param->CreateBound();
        }));

        ACE_TRY(boundOptBody, TransformExpectedOptional(m_OptBody,
        [](const std::shared_ptr<const BlockStmtNode>& t_body)
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

        return std::make_shared<const FunctionBoundNode>(
            selfSymbol,
            boundAttributes,
            boundOptSelf,
            boundParams,
            boundOptBody
        );
    }

    auto FunctionNode::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto FunctionNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto FunctionNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Function;
    }

    auto FunctionNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto FunctionNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
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
    
    auto FunctionNode::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto FunctionNode::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto FunctionNode::GetParams() const -> const std::vector<std::shared_ptr<const NormalParamVarNode>>&
    {
        return m_Params;
    }
}
