#include "BoundNode/Stmt/Assert.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "BoundNode/Stmt/If.hpp"
#include "BoundNode/Stmt/Group.hpp"
#include "BoundNode/Stmt/Block.hpp"
#include "BoundNode/Stmt/Exit.hpp"
#include "BoundNode/Expr//LogicalNegation.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Stmt
{
    Assert::Assert(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_condition
    ) : m_Condition{ t_condition }
    {
    }

    auto Assert::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto Assert::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto Assert::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Assert>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        if (!mchConvertedAndCheckedCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Assert>(
            mchConvertedAndCheckedCondition.Value
        );

        return CreateChanged(returnValue);
    }

    auto Assert::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Assert::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Group>>
    {
        const auto mchLoweredCondition = m_Condition->GetOrCreateLoweredExpr({});

        const auto condition = std::make_shared<const BoundNode::Expr::LogicalNegation>(
            mchLoweredCondition.Value
        );

        const auto bodyScope = GetScope()->GetOrCreateChild({});

        const auto exitStmt = std::make_shared<const BoundNode::Stmt::Exit>(
            bodyScope
        );

        const auto bodyStmt = std::make_shared<const BoundNode::Stmt::Block>(
            bodyScope,
            std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>>{ exitStmt }
        );

        const auto returnValue = std::make_shared<const BoundNode::Stmt::If>(
            GetScope(),
            std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>{ condition },
            std::vector<std::shared_ptr<const BoundNode::Stmt::Block>>{ bodyStmt }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    };

    auto Assert::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Assert::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
