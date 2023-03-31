#include "BoundNode/Statement/While.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Statement/Block.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Statement/Group.hpp"
#include "BoundNode/Statement/Jump/Normal.hpp"
#include "BoundNode/Statement/Jump/Conditional.hpp"
#include "BoundNode/Statement/Label.hpp"
#include "Scope.hpp"
#include "Symbol/Label.hpp"
#include "SpecialIdentifier.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "NativeSymbol.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Statement
{
    auto While::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Condition);
        AddChildren(children, m_Body);

        return children;
    }

    auto While::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::While>>>
    {
        ACE_TRY(mchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            TypeInfo{ NativeSymbol::Bool.GetSymbol(), ValueKind::R }
            ));

        ACE_TRY(mchCheckedBody, m_Body->GetOrCreateTypeChecked({ t_context.ParentFunctionTypeSymbol }));

        if (
            !mchConvertedAndCheckedCondition.IsChanged &&
            !mchCheckedBody.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::While>(
            m_Scope,
            mchConvertedAndCheckedCondition.Value,
            mchCheckedBody.Value
            );

        return CreateChanged(returnValue);
    }

    auto While::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>>
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

        auto startLabelSymbolOwned = std::make_unique<Symbol::Label>(
            m_Scope,
            SpecialIdentifier::CreateAnonymous()
            );

        auto* const startLabelSymbol = dynamic_cast<Symbol::Label*>(
            m_Scope->DefineSymbol(std::move(startLabelSymbolOwned)).Unwrap()
            );

        auto continueLabelSymbolOwned = std::make_unique<Symbol::Label>(
            m_Scope,
            SpecialIdentifier::CreateAnonymous()
            );

        auto* const continueLabelSymbol = dynamic_cast<Symbol::Label*>(
            m_Scope->DefineSymbol(std::move(continueLabelSymbolOwned)).Unwrap()
            );

        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> statements{};

        statements.push_back(std::make_shared<const BoundNode::Statement::Jump::Normal>(
            m_Scope,
            continueLabelSymbol
        ));

        statements.push_back(std::make_shared<const BoundNode::Statement::Label>(startLabelSymbol));

        statements.push_back(m_Body);

        statements.push_back(std::make_shared<const BoundNode::Statement::Label>(continueLabelSymbol));

        statements.push_back(std::make_shared<const BoundNode::Statement::Jump::Conditional>(
            m_Condition,
            startLabelSymbol
        ));

        const auto returnValue = std::make_shared<const BoundNode::Statement::Group>(
            m_Scope,
            statements
            );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered(t_context));
    }
}
