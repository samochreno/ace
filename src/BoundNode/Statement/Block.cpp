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
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    Block::Block(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>& t_statements
    ) : m_SelfScope{ t_selfScope },
        m_Statements{ t_statements }
    {
    }
    
    auto Block::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Block::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Statements);

        return children;
    }

    auto Block::GetOrCreateTypeChecked(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Block>>>
    {
        ACE_TRY(mchCheckedContent, TransformExpectedMaybeChangedVector(m_Statements,
        [&](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
        {
            return t_statement->GetOrCreateTypeCheckedStatement({
                t_context.ParentFunctionTypeSymbol
            });
        }));

        if (!mchCheckedContent.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Block>(
            m_SelfScope,
            mchCheckedContent.Value
        );
        return CreateChanged(returnValue);
    }

    auto Block::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Block::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Block>>
    {
        const auto mchLoweredStatements = TransformMaybeChangedVector(m_Statements,
        [](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
        {
            return t_statement->GetOrCreateLoweredStatement({});
        });

        if (!mchLoweredStatements.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Block>(
            m_SelfScope,
            mchLoweredStatements.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Block::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Block::Emit(Emitter& t_emitter) const -> void
    {
        const auto statements = CreateExpanded();
        t_emitter.EmitFunctionBodyStatements(statements);
    }

    auto Block::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        auto statements = m_Statements;

        const auto blockEnd = std::make_shared<const BoundNode::Statement::BlockEnd>(
            m_SelfScope
        );
        statements.push_back(blockEnd);

        return statements;
    }
}
