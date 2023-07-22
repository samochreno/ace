#include "BoundNodes/FunctionBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    FunctionBoundNode::FunctionBoundNode(
        const SrcLocation& srcLocation,
        FunctionSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
        const std::optional<const std::shared_ptr<const SelfParamVarBoundNode>>& optSelf,
        const std::vector<std::shared_ptr<const ParamVarBoundNode>>& params,
        const std::optional<std::shared_ptr<const BlockStmtBoundNode>>& optBody
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes },
        m_OptSelf{ optSelf },
        m_Params{ params },
        m_OptBody{ optBody }
    {
    }

    auto FunctionBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FunctionBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto FunctionBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
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

    auto FunctionBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const FunctionBoundNode>>>
    {
        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedOptSelf, TransformExpectedMaybeChangedOptional(m_OptSelf,
        [](const std::shared_ptr<const SelfParamVarBoundNode>& self)
        {
            return self->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedParams, TransformExpectedMaybeChangedVector(m_Params,
        [](const std::shared_ptr<const ParamVarBoundNode>& param)
        {
            return param->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedOptBody, TransformExpectedMaybeChangedOptional(m_OptBody,
        [&](const std::shared_ptr<const BlockStmtBoundNode>& body)
        {
            return body->GetOrCreateTypeChecked({ m_Symbol->GetType() });
        }));

        if (
            !mchCheckedAttributes.IsChanged &&
            !mchCheckedOptSelf.IsChanged && 
            !mchCheckedParams.IsChanged && 
            !mchCheckedOptBody.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const FunctionBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            mchCheckedAttributes.Value,
            mchCheckedOptSelf.Value,
            mchCheckedParams.Value,
            mchCheckedOptBody.Value
        ));
    }

    auto FunctionBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const FunctionBoundNode>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateLowered({});
        });
        
        const auto mchLoweredOptSelf = TransformMaybeChangedOptional(m_OptSelf,
        [](const std::shared_ptr<const SelfParamVarBoundNode>& self)
        {
            return self->GetOrCreateLowered({});
        });

        const auto mchLoweredParams = TransformMaybeChangedVector(m_Params,
        [](const std::shared_ptr<const ParamVarBoundNode>& param)
        {
            return param->GetOrCreateLowered({});
        });

        const auto mchLoweredOptBody = TransformMaybeChangedOptional(m_OptBody,
        [](const std::shared_ptr<const BlockStmtBoundNode>& body)
        {
            return body->GetOrCreateLowered({});
        });

        if (
            !mchLoweredAttributes.IsChanged && 
            !mchLoweredOptSelf.IsChanged &&
            !mchLoweredParams.IsChanged && 
            !mchLoweredOptBody.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const FunctionBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            mchLoweredAttributes.Value,
            mchLoweredOptSelf.Value,
            mchLoweredParams.Value,
            mchLoweredOptBody.Value
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
