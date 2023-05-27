#include "BoundNode/Statement/Block.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Statement/Label.hpp"
#include "BoundNode/Statement/Jump/Normal.hpp"
#include "BoundNode/Statement/Jump/Conditional.hpp"
#include "BoundNode/Statement/Return.hpp"
#include "BoundNode/Statement/Exit.hpp"
#include "BoundNode/Statement/Group.hpp"
#include "BoundNode/Statement/BlockEnd.hpp"
#include "BoundNode/Statement/Variable.hpp"
#include "Symbol/Label.hpp"
#include "Symbol/Variable/Local.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    auto Block::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Statements);

        return children;
    }

    auto Block::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Block>>>
    {
        ACE_TRY(mchCheckedContent, TransformExpectedMaybeChangedVector(m_Statements,
        [&](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
        {
            return t_statement->GetOrCreateTypeCheckedStatement({ t_context.ParentFunctionTypeSymbol });
        }));

        if (!mchCheckedContent.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Block>(
            m_SelfScope,
            mchCheckedContent.Value
        );

        return CreateChanged(returnValue);
    }

    auto Block::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Block>>>
    {
        ACE_TRY(mchLoweredStatements, TransformExpectedMaybeChangedVector(m_Statements,
        [](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
        {
            return t_statement->GetOrCreateLoweredStatement({});
        }));

        if (!mchLoweredStatements.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Block>(
            m_SelfScope,
            mchLoweredStatements.Value
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered(t_context));
    }

    auto Block::Emit(Emitter& t_emitter) const -> void
    {
        const auto statements = CreateExpanded();
        t_emitter.EmitFunctionBodyStatements(statements);
    }

    auto Block::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        auto statements = m_Statements;

        const auto blockEnd = std::make_shared<const BoundNode::Statement::BlockEnd>(m_SelfScope);
        statements.push_back(blockEnd);

        return statements;
    }

    auto Block::IsEndReachableWithoutReturn() const -> bool
    {
        struct ControlFlowStatement
        {
            ControlFlowStatement() = delete;

            enum class Kind
            {
                Label,
                NormalJump,
                ConditionalJump,
                Return,
                Exit,
            };

            class Data
            {
            public:
                struct Label
                {
                    Symbol::Label* LabelSymbol{};
                };

                struct Jump
                {
                    Jump() = delete;

                    struct Base
                    {
                        Symbol::Label* LabelSymbol{};
                    };

                    struct Normal : public Base
                    {
                    };

                    struct Conditional : public Base
                    {
                    };
                };

                struct Return
                {
                };

                struct Exit
                {
                };

                Data()
                {
                }

                static auto New(const BoundNode::Statement::IBase* const t_statement) -> Expected<Data>
                {
                    Data data{};

                    if (auto labelStatement = dynamic_cast<const BoundNode::Statement::Label*>(t_statement))
                    {
                        data.m_Kind = Kind::Label;
                        data.m_Label = { labelStatement->GetLabelSymbol() };
                    }
                    else if (auto normalJumpStatement = dynamic_cast<const BoundNode::Statement::Jump::Normal*>(t_statement))
                    {
                        data.m_Kind = Kind::NormalJump;
                        data.m_NormalJump = { normalJumpStatement->GetLabelSymbol() };
                    }
                    else if (auto conditionalJumpStatement = dynamic_cast<const BoundNode::Statement::Jump::Conditional*>(t_statement))
                    {
                        data.m_Kind = Kind::ConditionalJump;
                        data.m_ConditionalJump.LabelSymbol = { conditionalJumpStatement->GetLabelSymbol() };
                    }
                    else if (auto returnStatement = dynamic_cast<const BoundNode::Statement::Return*>(t_statement))
                    {
                        data.m_Kind = Kind::Return;
                    }
                    else if (auto exitStatement = dynamic_cast<const BoundNode::Statement::Exit*>(t_statement))
                    {
                        data.m_Kind = Kind::Exit;
                    }
                    else
                    {
                        ACE_TRY_UNREACHABLE();
                    }

                    return data;
                }

                auto GetKind() const -> Kind { return m_Kind; }

                auto AsLabel()              const -> const Label&               { return m_Label; }
                auto AsNormalJump()         const -> const Jump::Normal&        { return m_NormalJump; }
                auto AsConditionalJump()    const -> const Jump::Conditional&   { return m_ConditionalJump; }
                auto AsReturn()             const -> const Return&              { return m_Return; }
                auto AsExit()               const -> const Exit&                { return m_Exit; }

                Kind m_Kind{};

                union
                {
                    Label               m_Label;
                    Jump::Normal        m_NormalJump;
                    Jump::Conditional   m_ConditionalJump;
                    Return              m_Return;
                    Exit                m_Exit;
                };
            };
        };

        const auto statements = this->CreateExpanded();
        std::vector<ControlFlowStatement::Data> statementsData{};
        std::for_each(begin(statements), end(statements),
        [&](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
        {
            const auto expStatementData = ControlFlowStatement::Data::New(t_statement.get());
            
            if (expStatementData)
                statementsData.push_back(expStatementData.Unwrap());
        });

        const std::function<bool(const std::vector<ControlFlowStatement::Data>::iterator&, const std::vector<ControlFlowStatement::Data*>&)>
        isEndReachableWithoutReturn = [&](const std::vector<ControlFlowStatement::Data>::iterator& t_begin, const std::vector<ControlFlowStatement::Data*>& t_ends) -> bool
        {
            const auto findLabelStatementData = [&](const Symbol::Label* const t_labelSymbol) -> std::vector<ControlFlowStatement::Data>::iterator
            {
                auto labelIt = std::find_if(begin(statementsData), end(statementsData),
                [&](ControlFlowStatement::Data& t_statementData)
                {
                    if (t_statementData.GetKind() != ControlFlowStatement::Kind::Label)
                        return false;

                    if (t_statementData.AsLabel().LabelSymbol != t_labelSymbol)
                        return false;

                    return true;
                });

                ACE_ASSERT(labelIt != end(statementsData));
                return labelIt;
            };

            for (auto it = t_begin; it != end(statementsData); ++it)
            {
                auto& statementData = *it;

                const bool isEnd = std::find_if(begin(t_ends), end(t_ends),
                [&](ControlFlowStatement::Data* const t_end) 
                {
                    return &statementData == t_end;
                }) != end(t_ends);

                if (isEnd)
                    return false;

                switch (statementData.GetKind())
                {
                    case ControlFlowStatement::Kind::Label:
                        continue;

                    case ControlFlowStatement::Kind::NormalJump:
                    {
                        auto labelStatementDataIt = findLabelStatementData(statementData.AsNormalJump().LabelSymbol);
                        auto ends = t_ends;
                        ends.push_back(&statementData);
                        return isEndReachableWithoutReturn(labelStatementDataIt, ends);
                    }

                    case ControlFlowStatement::Kind::ConditionalJump:
                    {
                        auto labelStatementDataIt = findLabelStatementData(statementData.AsConditionalJump().LabelSymbol);

                        auto ends = t_ends;
                        ends.push_back(&statementData);
                        const bool whenTrue  = isEndReachableWithoutReturn(labelStatementDataIt, ends);

                        const bool whenFalse = isEndReachableWithoutReturn(it + 1, t_ends);

                        return whenTrue || whenFalse;
                    }

                    case ControlFlowStatement::Kind::Return:
                    case ControlFlowStatement::Kind::Exit:
                        return false;
                }

                ACE_UNREACHABLE();
            }

            return true;
        };

        return isEndReachableWithoutReturn(begin(statementsData), {});
    }
}
