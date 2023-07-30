#include "Nodes/FunctionNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Nodes/Vars/Params/SelfParamVarNode.hpp"
#include "Nodes/Vars/Params/NormalParamVarNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    FunctionNode::FunctionNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeNode>>& attributes,
        const AccessModifier accessModifier,
        const std::optional<std::shared_ptr<const SelfParamVarNode>>& optSelf,
        const std::vector<std::shared_ptr<const NormalParamVarNode>>& params,
        const std::optional<std::shared_ptr<const BlockStmtNode>>& optBody
    ) : m_SrcLocation{ srcLocation },
        m_SelfScope{ selfScope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_AccessModifier{ accessModifier },
        m_OptSelf{ optSelf },
        m_Params{ params },
        m_OptBody{ optBody }
    {
    }

    auto FunctionNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FunctionNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto FunctionNode::CollectChildren() const -> std::vector<const INode*>
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const FunctionNode>
    {
        const auto selfScope = scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const AttributeNode>> clonedAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(clonedAttributes),
            [&](const std::shared_ptr<const AttributeNode>& attribute)
            {
                return attribute->CloneInScope(scope);
            }
        );

        const auto clonedOptSelf = [&]() -> std::optional<std::shared_ptr<const SelfParamVarNode>>
        {
            if (!m_OptSelf.has_value())
            {
                return std::nullopt;
            }

            return m_OptSelf.value()->CloneInScope(selfScope);
        }();

        std::vector<std::shared_ptr<const NormalParamVarNode>> clonedParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(clonedParams),
            [&](const std::shared_ptr<const NormalParamVarNode>& param)
            {
                return param->CloneInScope(selfScope);
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
            m_SrcLocation,
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
        [](const std::shared_ptr<const AttributeNode>& attribute)
        {
            return attribute->CreateBound();
        }));

        ACE_TRY(boundOptSelf, TransformExpectedOptional(m_OptSelf,
        [](const std::shared_ptr<const SelfParamVarNode>& param)
        {
            return param->CreateBound();
        }));

        ACE_TRY(boundParams, TransformExpectedVector(m_Params,
        [](const std::shared_ptr<const NormalParamVarNode>& param)
        {
            return param->CreateBound();
        }));

        ACE_TRY(boundOptBody, TransformExpectedOptional(m_OptBody,
        [](const std::shared_ptr<const BlockStmtNode>& body)
        {
            return body->CreateBound();
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
            DiagnosticBag{},
            GetSrcLocation(),
            selfSymbol,
            boundAttributes,
            boundOptSelf,
            boundParams,
            boundOptBody
        );
    }

    auto FunctionNode::GetName() const -> const Ident&
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

    auto FunctionNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        DiagnosticBag diagnostics{};

        const auto symbolCategory = m_OptSelf.has_value() ?
            SymbolCategory::Instance :
            SymbolCategory::Static;

        const auto expTypeSymbol = m_SelfScope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        );
        diagnostics.Add(expTypeSymbol);

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<FunctionSymbol>(
                m_SelfScope,
                m_Name,
                symbolCategory,
                m_AccessModifier,
                expTypeSymbol.UnwrapOr(GetCompilation()->ErrorTypeSymbol)
            ),
            diagnostics,
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
