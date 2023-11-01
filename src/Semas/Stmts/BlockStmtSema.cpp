#include "Semas/Stmts/BlockStmtSema.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Stmts/LabelStmtSema.hpp"
#include "Semas/Stmts/Jumps/NormalJumpStmtSema.hpp"
#include "Semas/Stmts/Jumps/ConditionalJumpStmtSema.hpp"
#include "Semas/Stmts/RetStmtSema.hpp"
#include "Semas/Stmts/ExitStmtSema.hpp"
#include "Semas/Stmts/GroupStmtSema.hpp"
#include "Semas/Stmts/BlockEndStmtSema.hpp"
#include "Semas/Stmts/VarStmtSema.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Emitter.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    BlockStmtSema::BlockStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        const std::vector<std::shared_ptr<const IStmtSema>>& stmts
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope },
        m_Stmts{ stmts }
    {
    }

    auto BlockStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }
    
    auto BlockStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto BlockStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const BlockStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const IStmtSema>> checkedStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(checkedStmts),
            [&](const std::shared_ptr<const IStmtSema>& stmt)
            {
                return diagnostics.Collect(stmt->CreateTypeCheckedStmt({
                    context.ParentFunctionTypeSymbol
                }));
            }
        );

        if (checkedStmts == m_Stmts)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const BlockStmtSema>(
                GetSrcLocation(),
                m_BodyScope,
                checkedStmts
            ),
            std::move(diagnostics),
        };
    }

    auto BlockStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto BlockStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const BlockStmtSema>
    {
        std::vector<std::shared_ptr<const IStmtSema>> loweredStmts{};
        std::transform(
            begin(m_Stmts),
            end  (m_Stmts),
            back_inserter(loweredStmts),
            [&](const std::shared_ptr<const IStmtSema>& stmt)
            {
                return stmt->CreateLoweredStmt({});
            }
        );

        if (loweredStmts == m_Stmts)
        {
            return shared_from_this();
        }

        return std::make_shared<const BlockStmtSema>(
            GetSrcLocation(),
            m_BodyScope,
            loweredStmts
        )->CreateLowered({});
    }

    auto BlockStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto BlockStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Stmts);
    }

    auto BlockStmtSema::Emit(Emitter& emitter) const -> void
    {
        const auto stmts = CreateExpanded();
        emitter.EmitFunctionBlockStmts(stmts);
    }

    auto BlockStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        std::vector<ControlFlowNode> nodes{};
        std::for_each(
            begin(m_Stmts),
            end  (m_Stmts),
            [&](const std::shared_ptr<const IStmtSema>& stmt)
            {
                const auto stmtNodes = stmt->CreateControlFlowNodes();
                nodes.insert(end(nodes), begin(stmtNodes), end(stmtNodes));
            }
        );

        return nodes;
    }

    auto BlockStmtSema::CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtSema>>
    {
        auto stmts = m_Stmts;

        const auto blockEnd = std::make_shared<const BlockEndStmtSema>(
            GetSrcLocation().CreateLast(),
            m_BodyScope
        );
        stmts.push_back(blockEnd);

        return stmts;
    }
}
