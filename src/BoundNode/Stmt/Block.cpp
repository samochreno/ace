#include "BoundNode/Stmt/Block.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "BoundNode/Stmt/Base.hpp"
#include "BoundNode/Stmt/Label.hpp"
#include "BoundNode/Stmt/Jump/Normal.hpp"
#include "BoundNode/Stmt/Jump/Conditional.hpp"
#include "BoundNode/Stmt/Return.hpp"
#include "BoundNode/Stmt/Exit.hpp"
#include "BoundNode/Stmt/Group.hpp"
#include "BoundNode/Stmt/BlockEnd.hpp"
#include "BoundNode/Stmt/Var.hpp"
#include "Symbol/Label.hpp"
#include "Symbol/Var/Local.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Stmt
{
    Block::Block(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>>& t_stmts
    ) : m_SelfScope{ t_selfScope },
        m_Stmts{ t_stmts }
    {
    }
    
    auto Block::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Block::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto Block::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Block>>>
    {
        ACE_TRY(mchCheckedContent, TransformExpectedMaybeChangedVector(m_Stmts,
        [&](const std::shared_ptr<const BoundNode::Stmt::IBase>& t_stmt)
        {
            return t_stmt->GetOrCreateTypeCheckedStmt({
                t_context.ParentFunctionTypeSymbol
            });
        }));

        if (!mchCheckedContent.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Block>(
            m_SelfScope,
            mchCheckedContent.Value
        );
        return CreateChanged(returnValue);
    }

    auto Block::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Block::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Block>>
    {
        const auto mchLoweredStmts = TransformMaybeChangedVector(m_Stmts,
        [](const std::shared_ptr<const BoundNode::Stmt::IBase>& t_stmt)
        {
            return t_stmt->GetOrCreateLoweredStmt({});
        });

        if (!mchLoweredStmts.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Stmt::Block>(
            m_SelfScope,
            mchLoweredStmts.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Block::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Block::Emit(Emitter& t_emitter) const -> void
    {
        const auto stmts = CreateExpanded();
        t_emitter.EmitFunctionBodyStmts(stmts);
    }

    auto Block::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        auto stmts = m_Stmts;

        const auto blockEnd = std::make_shared<const BoundNode::Stmt::BlockEnd>(
            m_SelfScope
        );
        stmts.push_back(blockEnd);

        return stmts;
    }
}
