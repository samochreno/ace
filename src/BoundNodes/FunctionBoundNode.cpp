#include "BoundNodes/FunctionBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SourceLocation.hpp"
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
        const SourceLocation& t_sourceLocation,
        FunctionSymbol* const t_symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& t_attributes,
        const std::optional<const std::shared_ptr<const SelfParamVarBoundNode>>& t_optSelf,
        const std::vector<std::shared_ptr<const ParamVarBoundNode>>& t_params,
        const std::optional<std::shared_ptr<const BlockStmtBoundNode>>& t_optBody
    ) : m_SourceLocation{ t_sourceLocation },
        m_Symbol{ t_symbol },
        m_Attributes{ t_attributes },
        m_OptSelf{ t_optSelf },
        m_Params{ t_params },
        m_OptBody{ t_optBody }
    {
    }

    auto FunctionBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const FunctionBoundNode>>>
    {
        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& t_attribute)
        {
            return t_attribute->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedOptSelf, TransformExpectedMaybeChangedOptional(m_OptSelf,
        [](const std::shared_ptr<const SelfParamVarBoundNode>& t_self)
        {
            return t_self->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedParams, TransformExpectedMaybeChangedVector(m_Params,
        [](const std::shared_ptr<const ParamVarBoundNode>& t_param)
        {
            return t_param->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedOptBody, TransformExpectedMaybeChangedOptional(m_OptBody,
        [&](const std::shared_ptr<const BlockStmtBoundNode>& t_body)
        {
            return t_body->GetOrCreateTypeChecked({ m_Symbol->GetType() });
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
            GetSourceLocation(),
            m_Symbol,
            mchCheckedAttributes.Value,
            mchCheckedOptSelf.Value,
            mchCheckedParams.Value,
            mchCheckedOptBody.Value
        ));
    }

    auto FunctionBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const FunctionBoundNode>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& t_attribute)
        {
            return t_attribute->GetOrCreateLowered({});
        });
        
        const auto mchLoweredOptSelf = TransformMaybeChangedOptional(m_OptSelf,
        [](const std::shared_ptr<const SelfParamVarBoundNode>& t_self)
        {
            return t_self->GetOrCreateLowered({});
        });

        const auto mchLoweredParams = TransformMaybeChangedVector(m_Params,
        [](const std::shared_ptr<const ParamVarBoundNode>& t_param)
        {
            return t_param->GetOrCreateLowered({});
        });

        const auto mchLoweredOptBody = TransformMaybeChangedOptional(m_OptBody,
        [](const std::shared_ptr<const BlockStmtBoundNode>& t_body)
        {
            return t_body->GetOrCreateLowered({});
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
            GetSourceLocation(),
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
