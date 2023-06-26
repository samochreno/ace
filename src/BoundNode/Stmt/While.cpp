#include "BoundNode/Stmt/While.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Block.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "BoundNode/Stmt/Group.hpp"
#include "BoundNode/Stmt/Jump/Normal.hpp"
#include "BoundNode/Stmt/Jump/Conditional.hpp"
#include "BoundNode/Stmt/Label.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "SpecialIdentifier.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Stmt
{
    While::While(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_condition,
        const std::shared_ptr<const BoundNode::Stmt::Block>& t_body
    ) : m_Scope{ t_scope },
        m_Condition{ t_condition },
        m_Body{ t_body }
    {
    }

    auto While::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto While::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Condition);
        AddChildren(children, m_Body);

        return children;
    }

    auto While::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::While>>>
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

        ACE_TRY(mchCheckedBody, m_Body->GetOrCreateTypeChecked({
            t_context.ParentFunctionTypeSymbol
        }));

        if (
            !mchConvertedAndCheckedCondition.IsChanged &&
            !mchCheckedBody.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::While>(
            m_Scope,
            mchConvertedAndCheckedCondition.Value,
            mchCheckedBody.Value
        );
        return CreateChanged(returnValue);
    }

    auto While::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto While::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Group>>
    {
        // From:
        // while condition {
        //     body;
        // }
        // 
        // To:
        // goto continue;
        // start:
        // body;
        // continue:
        // gotoif condition start;

        auto startLabelSymbolOwned = std::make_unique<LabelSymbol>(
            m_Scope,
            SpecialIdentifier::CreateAnonymous()
        );

        auto* const startLabelSymbol = dynamic_cast<LabelSymbol*>(
            m_Scope->DefineSymbol(std::move(startLabelSymbolOwned)).Unwrap()
        );

        auto continueLabelSymbolOwned = std::make_unique<LabelSymbol>(
            m_Scope,
            SpecialIdentifier::CreateAnonymous()
        );

        auto* const continueLabelSymbol = dynamic_cast<LabelSymbol*>(
            m_Scope->DefineSymbol(std::move(continueLabelSymbolOwned)).Unwrap()
        );

        std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>> stmts{};

        stmts.push_back(std::make_shared<const BoundNode::Stmt::Jump::Normal>(
            m_Scope,
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const BoundNode::Stmt::Label>(
            startLabelSymbol
        ));

        stmts.push_back(m_Body);

        stmts.push_back(std::make_shared<const BoundNode::Stmt::Label>(
            continueLabelSymbol
        ));

        stmts.push_back(std::make_shared<const BoundNode::Stmt::Jump::Conditional>(
            m_Condition,
            startLabelSymbol
        ));

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Group>(
            m_Scope,
            stmts
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto While::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto While::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
