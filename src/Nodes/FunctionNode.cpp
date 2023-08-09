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

    auto FunctionNode::CreateBound() const -> Diagnosed<std::shared_ptr<const FunctionBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const AttributeBoundNode>> boundAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(boundAttributes),
            [&](const std::shared_ptr<const AttributeNode>& attribute)
            {
                return diagnostics.Collect(attribute->CreateBound());
            }
        );

        std::optional<std::shared_ptr<const SelfParamVarBoundNode>> boundOptSelf{};
        if (m_OptSelf.has_value())
        {
            boundOptSelf =
                diagnostics.Collect(m_OptSelf.value()->CreateBound());
        }

        std::vector<std::shared_ptr<const NormalParamVarBoundNode>> boundParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(boundParams),
            [&](const std::shared_ptr<const NormalParamVarNode>& param)
            {
                return diagnostics.Collect(param->CreateBound());
            }
        );

        std::optional<std::shared_ptr<const BlockStmtBoundNode>> boundOptBody{};
        if (m_OptBody.has_value())
        {
            boundOptBody =
                diagnostics.Collect(m_OptBody.value()->CreateBound());
        }

        auto* const selfSymbol = GetScope()->ExclusiveResolveSymbol<FunctionSymbol>(
            m_Name,
            m_SelfScope->CollectImplTemplateArgs(),
            m_SelfScope->CollectTemplateArgs()
        ).Unwrap();

        return Diagnosed
        {
            std::make_shared<const FunctionBoundNode>(
                GetSrcLocation(),
                selfSymbol,
                boundAttributes,
                boundOptSelf,
                boundParams,
                boundOptBody
            ),
            std::move(diagnostics),
        };
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
        auto diagnostics = DiagnosticBag::Create();

        const auto symbolCategory = m_OptSelf.has_value() ?
            SymbolCategory::Instance :
            SymbolCategory::Static;

        const auto optTypeSymbol = diagnostics.Collect(m_SelfScope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<FunctionSymbol>(
                m_SelfScope,
                m_Name,
                symbolCategory,
                m_AccessModifier,
                typeSymbol
            ),
            std::move(diagnostics),
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
