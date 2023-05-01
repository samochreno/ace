#include "BoundNode/Statement/Assignment/Compound.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "BoundNode/Expression//Reference.hpp"
#include "BoundNode/Expression//Dereference.hpp"
#include "BoundNode/Expression/VariableReference/Static.hpp"
#include "BoundNode/Expression/VariableReference/Instance.hpp"
#include "BoundNode/Expression/BinaryUser.hpp"
#include "BoundNode/Statement/Group.hpp"
#include "BoundNode/Statement/Block.hpp"
#include "BoundNode/Statement/Expression.hpp"
#include "BoundNode/Statement/Assignment/Normal.hpp"
#include "BoundNode/Statement/Variable.hpp"
#include "Symbol/Variable/Local.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::BoundNode::Statement::Assignment
{
    auto Compound::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto Compound::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Assignment::Compound>>>
    {
        const auto argumentTypeInfos = m_OperatorSymbol->CollectArgumentTypeInfos();
        ACE_ASSERT(argumentTypeInfos.size() == 2);

        ACE_TRY(mchConvertedAndCheckedLHSExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpression,
            TypeInfo{ argumentTypeInfos.at(0).Symbol, ValueKind::L }
        ));

        ACE_TRY(mchConvertedAndCheckedRHSExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpression,
            TypeInfo{ argumentTypeInfos.at(1).Symbol, ValueKind::R }
        ));

        if (
            !mchConvertedAndCheckedLHSExpression.IsChanged &&
            !mchConvertedAndCheckedRHSExpression.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Assignment::Compound>(
            mchConvertedAndCheckedLHSExpression.Value,
            mchConvertedAndCheckedRHSExpression.Value,
            m_OperatorSymbol
        );

        return CreateChanged(returnValue);
    }

    auto Compound::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>>
    {
        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> statements{};

        const bool isStaticVariableReferenceExpression = [&]() -> bool
        {
            auto* nextExpression = m_LHSExpression.get();
            while (const auto* const expression = dynamic_cast<const BoundNode::Expression::Dereference*>(nextExpression))
            {
                nextExpression = expression->GetExpression().get();
            }

            return dynamic_cast<const BoundNode::Expression::VariableReference::Static*>(nextExpression) != nullptr;
        }();

        if (isStaticVariableReferenceExpression)
        {
            // From:
            // lhs += rhs;
            // 
            // To:
            // lhs = lhs + rhs;

            const auto userBinaryExpression = std::make_shared<const BoundNode::Expression::BinaryUser>(
                m_LHSExpression,
                m_RHSExpression,
                m_OperatorSymbol
            );

            const auto assignmentStatement = std::make_shared<const BoundNode::Statement::Assignment::Normal>(
                m_LHSExpression,
                userBinaryExpression
            );
            statements.push_back(assignmentStatement);
        }
        else if (auto instanceVariableReferenceExpression = dynamic_cast<const BoundNode::Expression::VariableReference::Instance*>(m_LHSExpression.get()))
        {
            // From:
            // lhs.variable += rhs;
            // 
            // To: (if lhs is L-value)
            // {
            //     tmp_ref: &auto = lhs;
            //     tmp_ref.variable = tmp_ref.field + rhs;
            // }

            // To: (if lhs is R-value)
            // {
            //     tmp: auto = lhs;
            //     tmp_ref: &auto = tmp;
            //     tmp_ref.variable = tmp_ref.field + rhs;
            // }

            std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> blockStatements{};
            Scope* const blockScope = GetScope()->GetOrCreateChild({});

            const auto tmpRefExpression = [&]() -> std::shared_ptr<const BoundNode::Expression::IBase>
            {
                const auto expression = instanceVariableReferenceExpression->GetExpression();
                
                switch (expression->GetTypeInfo().ValueKind)
                {
                    case ValueKind::L:
                    {
                        if (expression->GetTypeInfo().Symbol->IsReference())
                        {
                            return expression;
                        }

                        return std::make_shared<const BoundNode::Expression::Reference>(expression);
                    }

                    case ValueKind::R:
                    {
                        ACE_ASSERT(!expression->GetTypeInfo().Symbol->IsReference());

                        auto tmpVariableSymbolOwned = std::make_unique<Symbol::Variable::Local>(
                            blockScope,
                            SpecialIdentifier::CreateAnonymous(),
                            expression->GetTypeInfo().Symbol
                        );

                        auto* const tmpVariableSymbol = dynamic_cast<Symbol::Variable::Local*>(
                            blockScope->DefineSymbol(std::move(tmpVariableSymbolOwned)).Unwrap()
                            );
                        ACE_ASSERT(tmpVariableSymbol);

                        const auto tmpVariableStatement = std::make_shared<const BoundNode::Statement::Variable>(
                            tmpVariableSymbol,
                            expression
                        );
                        blockStatements.push_back(tmpVariableStatement);

                        const auto tmpVariableReferenceExpression = std::make_shared<const BoundNode::Expression::VariableReference::Static>(
                            blockScope,
                            tmpVariableSymbol
                        );

                        return std::make_shared<const BoundNode::Expression::Reference>(tmpVariableReferenceExpression);
                    }

                    default:
                    {
                        ACE_UNREACHABLE();
                    }
                }
            }();

            auto tmpRefVariableSymbolOwned = std::make_unique<Symbol::Variable::Local>(
                blockScope,
                SpecialIdentifier::CreateAnonymous(),
                tmpRefExpression->GetTypeInfo().Symbol
            );

            auto* const tmpRefVariableSymbol = dynamic_cast<Symbol::Variable::Local*>(
                blockScope->DefineSymbol(std::move(tmpRefVariableSymbolOwned)).Unwrap()
                );
            ACE_ASSERT(tmpRefVariableSymbol);

            const auto tmpRefVariableStatement = std::make_shared<const BoundNode::Statement::Variable>(
                tmpRefVariableSymbol,
                tmpRefExpression
            );
            blockStatements.push_back(tmpRefVariableStatement);

            const auto tmpRefVariableReferenceExpression = std::make_shared<const BoundNode::Expression::VariableReference::Static>(
                blockScope,
                tmpRefVariableSymbol
            );

            const auto tmpRefVariableFieldReferenceExpression = std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
                tmpRefVariableReferenceExpression,
                instanceVariableReferenceExpression->GetVariableSymbol()
            );

            const auto userBinaryExpression = std::make_shared<const BoundNode::Expression::BinaryUser>(
                tmpRefVariableFieldReferenceExpression,
                m_RHSExpression,
                m_OperatorSymbol
            );

            const auto assignmentStatement = std::make_shared<const BoundNode::Statement::Assignment::Normal>(
                tmpRefVariableFieldReferenceExpression,
                userBinaryExpression
            );
            blockStatements.push_back(assignmentStatement);

            const auto blockStatement = std::make_shared<const BoundNode::Statement::Block>(
                blockScope,
                blockStatements
            );
            statements.push_back(blockStatement);
        }
        else
        {
            ACE_UNREACHABLE();
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Group>(
            GetScope(),
            statements
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }
}
