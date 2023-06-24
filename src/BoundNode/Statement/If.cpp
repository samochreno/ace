#include "BoundNode/Statement/If.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>

#include "BoundNode/Statement/Block.hpp"
#include "BoundNode/Statement/Group.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression//LogicalNegation.hpp"
#include "BoundNode/Statement/Label.hpp"
#include "BoundNode/Statement/Jump/Conditional.hpp"
#include "BoundNode/Statement/Jump/Normal.hpp"
#include "Symbol/Label.hpp"
#include "SpecialIdentifier.hpp"
#include "Asserts.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Statement
{
    If::If(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<std::shared_ptr<const BoundNode::Expression::IBase>>& t_conditions,
        const std::vector<std::shared_ptr<const BoundNode::Statement::Block>>& t_bodies
    ) : m_Scope{ t_scope },
        m_Conditions{ t_conditions },
        m_Bodies{ t_bodies }
    {
    }

    auto If::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto If::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Conditions);
        AddChildren(children, m_Bodies);

        return children;
    }

    auto If::GetOrCreateTypeChecked(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::If>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedConditions, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Conditions,
            typeInfo
        ));

        ACE_TRY(mchCheckedBodies, TransformExpectedMaybeChangedVector(m_Bodies,
        [&](const std::shared_ptr<const BoundNode::Statement::Block>& t_body)
        {
            return t_body->GetOrCreateTypeChecked({
                t_context.ParentFunctionTypeSymbol
            });
        }));

        if (
            !mchConvertedAndCheckedConditions.IsChanged &&
            !mchCheckedBodies.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::If>(
            m_Scope,
            mchConvertedAndCheckedConditions.Value,
            mchCheckedBodies.Value
        );
        return CreateChanged(returnValue);
    }

    auto If::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto If::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>
    {
        // From:
        // if condition_0 {
        //     body_0;
        // } elif condition_1 {
        //     body_1;
        // } else {
        //     body_2;
        // }
        // 
        // To: 
        // gotoif !condition_0 label_0;
        // body_0;
        // goto label_2;
        // 
        // label_0:
        // gotoif !condition_1 label_1;
        // body_1;
        // goto label_2;
        // 
        // label_1:
        // body_2;
        // 
        // label_2:

        const bool hasElse = m_Bodies.size() > m_Conditions.size();

        std::vector<Symbol::Label*> labelSymbols{};
        const size_t labelCount = m_Conditions.size() + (hasElse ? 1 : 0);
        for (size_t i = 0; i < labelCount; i++)
        {
            auto labelSymbolOwned = std::make_unique<Symbol::Label>(
                m_Scope,
                SpecialIdentifier::CreateAnonymous()
            );

            auto* const labelSymbol = dynamic_cast<Symbol::Label*>(
                m_Scope->DefineSymbol(std::move(labelSymbolOwned)).Unwrap()
            );

            labelSymbols.push_back(labelSymbol);
        }

        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> statements{};

        for (size_t i = 0; i < m_Conditions.size(); i++)
        {
            const bool isFirstBody = i == 0;
            const bool isLastBody  = i == (m_Bodies.size() - 1);

            if (!isFirstBody)
            {
                statements.push_back(std::make_shared<const BoundNode::Statement::Label>(
                    labelSymbols.at(i - 1)
                ));
            }

            const auto condition = std::make_shared<const BoundNode::Expression::LogicalNegation>(
                m_Conditions.at(i)
            );

            statements.push_back(std::make_shared<const BoundNode::Statement::Jump::Conditional>(
                condition,
                labelSymbols.at(i)
            ));

            statements.push_back(m_Bodies.at(i));

            if (!isLastBody)
            {
                statements.push_back(std::make_shared<const BoundNode::Statement::Jump::Normal>(
                    m_Scope,
                    labelSymbols.back()
                ));
            }
        }

        if (hasElse)
        {
            statements.push_back(std::make_shared<const BoundNode::Statement::Label>(
                labelSymbols.rbegin()[1]
            ));

            statements.push_back(m_Bodies.back());
        }

        statements.push_back(std::make_shared<const BoundNode::Statement::Label>(
            labelSymbols.back()
        ));

        const auto returnValue = std::make_shared<const BoundNode::Statement::Group>(
            m_Scope,
            statements
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto If::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto If::Emit(Emitter& t_emitter) const -> void
    {
        ACE_UNREACHABLE();
    }
}
