#include "BoundNode/Function.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "BoundNode/Attribute.hpp"
#include "BoundNode/Var/Param/Self.hpp"
#include "BoundNode/Var/Param/Normal.hpp"
#include "BoundNode/Stmt/Block.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    Function::Function(
        FunctionSymbol* const t_symbol,
        const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes,
        const std::optional<const std::shared_ptr<const BoundNode::Var::Param::Self>>& t_optSelf,
        const std::vector<std::shared_ptr<const BoundNode::Var::Param::Normal>>& t_params,
        const std::optional<std::shared_ptr<const BoundNode::Stmt::Block>>& t_optBody
    ) : m_Symbol{ t_symbol },
        m_Attributes{ t_attributes },
        m_OptSelf{ t_optSelf },
        m_Params{ t_params },
        m_OptBody{ t_optBody }
    {
    }

    auto Function::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto Function::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

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

    auto Function::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Function>>>
    {
        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedOptSelf, TransformExpectedMaybeChangedOptional(m_OptSelf,
        [](const std::shared_ptr<const BoundNode::Var::Param::Self>& t_self)
        {
            return t_self->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedParams, TransformExpectedMaybeChangedVector(m_Params,
        [](const std::shared_ptr<const BoundNode::Var::Param::Normal>& t_param)
        {
            return t_param->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedOptBody, TransformExpectedMaybeChangedOptional(m_OptBody,
        [&](const std::shared_ptr<const BoundNode::Stmt::Block>& t_body)
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

        const auto returnValue = std::make_shared<const BoundNode::Function>(
            m_Symbol,
            mchCheckedAttributes.Value,
            mchCheckedOptSelf.Value,
            mchCheckedParams.Value,
            mchCheckedOptBody.Value
        );
        return CreateChanged(returnValue);
    }

    auto Function::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Function>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateLowered({});
        });
        
        const auto mchLoweredOptSelf = TransformMaybeChangedOptional(m_OptSelf,
        [](const std::shared_ptr<const BoundNode::Var::Param::Self>& t_self)
        {
            return t_self->GetOrCreateLowered({});
        });

        const auto mchLoweredParams = TransformMaybeChangedVector(m_Params,
        [](const std::shared_ptr<const BoundNode::Var::Param::Normal>& t_param)
        {
            return t_param->GetOrCreateLowered({});
        });

        const auto mchLoweredOptBody = TransformMaybeChangedOptional(m_OptBody,
        [](const std::shared_ptr<const BoundNode::Stmt::Block>& t_body)
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

        const auto returnValue = std::make_shared<const BoundNode::Function>(
            m_Symbol,
            mchLoweredAttributes.Value,
            mchLoweredOptSelf.Value,
            mchLoweredParams.Value,
            mchLoweredOptBody.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Function::GetSymbol() const -> FunctionSymbol*
    {
        return m_Symbol;
    }

    auto Function::GetBody() const -> std::optional<std::shared_ptr<const BoundNode::Stmt::Block>>
    {
        return m_OptBody;
    }
}
