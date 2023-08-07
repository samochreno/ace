#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/Jumps/ConditionalJumpStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockEndStmtBoundNode.hpp"
#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Emitter.hpp"
#include "CFA.hpp"

namespace Ace
{
    BlockStmtBoundNode::BlockStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope,
        const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
    ) : m_SrcLocation{ srcLocation },
        m_SelfScope{ selfScope },
        m_Stmts{ stmts }
    {
    }

    auto BlockStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }
    
    auto BlockStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto BlockStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Stmts);

        return children;
    }

    auto BlockStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const BlockStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const IStmtBoundNode>> checkedStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(checkedStmts),
            [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
            {
                return diagnostics.Collect(stmt->CreateTypeCheckedStmt({
                    context.ParentFunctionTypeSymbol
                }));
            }
        );

        if (checkedStmts == m_Stmts)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const BlockStmtBoundNode>(
                GetSrcLocation(),
                m_SelfScope,
                checkedStmts
            ),
            diagnostics,
        };
    }

    auto BlockStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto BlockStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const BlockStmtBoundNode>
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> loweredStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(loweredStmts),
            [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
            {
                return stmt->CreateLoweredStmt({});
            }
        );

        if (loweredStmts == m_Stmts)
        {
            return shared_from_this();
        }

        return std::make_shared<const BlockStmtBoundNode>(
            GetSrcLocation(),
            m_SelfScope,
            loweredStmts
        )->CreateLowered({});
    }

    auto BlockStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto BlockStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        const auto stmts = CreateExpanded();
        emitter.EmitFunctionBodyStmts(stmts);
    }

    auto BlockStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        std::vector<CFANode> nodes{};
        std::for_each(
            begin(m_Stmts),
            end  (m_Stmts),
            [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
            {
                const auto stmtNodes = stmt->CreateCFANodes();
                nodes.insert(end(nodes), begin(stmtNodes), end(stmtNodes));
            }
        );

        return nodes;
    }

    auto BlockStmtBoundNode::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        auto stmts = m_Stmts;

        const auto blockEnd = std::make_shared<const BlockEndStmtBoundNode>(
            GetSrcLocation().CreateLast(),
            m_SelfScope
        );
        stmts.push_back(blockEnd);

        return stmts;
    }
}
