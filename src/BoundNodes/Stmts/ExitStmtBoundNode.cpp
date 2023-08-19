#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "CFA.hpp"

namespace Ace
{
    ExitStmtBoundNode::ExitStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope }
    {
    }

    auto ExitStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExitStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ExitStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto ExitStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ExitStmtBoundNode>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto ExitStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto ExitStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ExitStmtBoundNode>
    {
        return shared_from_this();
    }
    
    auto ExitStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto ExitStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        const auto srcLocation = GetSrcLocation();
        const auto location = srcLocation.Buffer->FormatLocation(srcLocation);

        emitter.EmitPrintf(emitter.EmitString(
            "Program aborted at " + location
        ));

        auto* const argValue = llvm::ConstantInt::get(
            emitter.GetC().GetTypes().GetInt(),
            -1,
            true
        );

        emitter.GetBlockBuilder().Builder.CreateCall(
            emitter.GetC().GetFunctions().GetExit(),
            { argValue }
        );

        emitter.GetBlockBuilder().Builder.CreateUnreachable();
    }

    auto ExitStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return std::vector{ CFANode{ CFANodeKind::Exit } };
    }
}
