#include "BoundNodes/FunctionBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    FunctionBoundNode::FunctionBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        FunctionSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
        const std::optional<const std::shared_ptr<const SelfParamVarBoundNode>>& optSelf,
        const std::vector<std::shared_ptr<const NormalParamVarBoundNode>>& params,
        const std::optional<std::shared_ptr<const BlockStmtBoundNode>>& optBody
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes },
        m_OptSelf{ optSelf },
        m_Params{ params },
        m_OptBody{ optBody }
    {
    }

    auto FunctionBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto FunctionBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FunctionBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto FunctionBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

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

    auto FunctionBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const FunctionBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const FunctionBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetSymbol(),
            m_Attributes,
            m_OptSelf,
            m_Params,
            GetBody()
        );
    }

    auto FunctionBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const FunctionBoundNode>>>
    {
        ACE_TRY(cchCheckedAttributes, TransformExpectedCacheableVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(cchCheckedOptSelf, TransformExpectedCacheableOptional(m_OptSelf,
        [](const std::shared_ptr<const SelfParamVarBoundNode>& self)
        {
            return self->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(cchCheckedParams, TransformExpectedCacheableVector(m_Params,
        [](const std::shared_ptr<const NormalParamVarBoundNode>& param)
        {
            return param->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(cchCheckedOptBody, TransformExpectedCacheableOptional(m_OptBody,
        [&](const std::shared_ptr<const BlockStmtBoundNode>& body)
        {
            return body->GetOrCreateTypeChecked({ m_Symbol->GetType() });
        }));

        if (
            !cchCheckedAttributes.IsChanged &&
            !cchCheckedOptSelf.IsChanged && 
            !cchCheckedParams.IsChanged && 
            !cchCheckedOptBody.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const FunctionBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchCheckedAttributes.Value,
            cchCheckedOptSelf.Value,
            cchCheckedParams.Value,
            cchCheckedOptBody.Value
        ));
    }

    auto FunctionBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const FunctionBoundNode>>
    {
        const auto cchLoweredAttributes = TransformCacheableVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateLowered({});
        });
        
        const auto cchLoweredOptSelf = TransformCacheableOptional(m_OptSelf,
        [](const std::shared_ptr<const SelfParamVarBoundNode>& self)
        {
            return self->GetOrCreateLowered({});
        });

        const auto cchLoweredParams = TransformCacheableVector(m_Params,
        [](const std::shared_ptr<const NormalParamVarBoundNode>& param)
        {
            return param->GetOrCreateLowered({});
        });

        const auto cchLoweredOptBody = TransformCacheableOptional(m_OptBody,
        [](const std::shared_ptr<const BlockStmtBoundNode>& body)
        {
            return body->GetOrCreateLowered({});
        });

        if (
            !cchLoweredAttributes.IsChanged && 
            !cchLoweredOptSelf.IsChanged &&
            !cchLoweredParams.IsChanged && 
            !cchLoweredOptBody.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const FunctionBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchLoweredAttributes.Value,
            cchLoweredOptSelf.Value,
            cchLoweredParams.Value,
            cchLoweredOptBody.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto FunctionBoundNode::GetSymbol() const -> FunctionSymbol*
    {
        return m_Symbol;
    }

    auto FunctionBoundNode::GetBody() const -> std::optional<std::shared_ptr<const BlockStmtBoundNode>>
    {
        return m_OptBody;
    }
}
