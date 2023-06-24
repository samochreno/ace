#include "BoundNode/Expression/SizeOf.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "ExpressionDropData.hpp"

namespace Ace::BoundNode::Expression
{
    SizeOf::SizeOf(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Type::IBase* const t_typeSymbol
    ) : m_Scope{ t_scope },
        m_TypeSymbol{ t_typeSymbol }
    {
    }

    auto SizeOf::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SizeOf::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto SizeOf::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::SizeOf>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto SizeOf::GetOrCreateTypeCheckedExpression(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto SizeOf::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::SizeOf>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto SizeOf::GetOrCreateLoweredExpression(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto SizeOf::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        auto* const intTypeSymbol = GetCompilation()->Natives->Int.GetSymbol();
        auto* const intType = t_emitter.GetIRType(intTypeSymbol);
        auto* const type = t_emitter.GetIRType(m_TypeSymbol);

        auto* const value = llvm::ConstantInt::get(
            intType,
            t_emitter.GetModule().getDataLayout().getTypeAllocSize(type)
        );

        auto* const allocaInst =
            t_emitter.GetBlockBuilder().Builder.CreateAlloca(intType);
        temporaries.emplace_back(allocaInst, intTypeSymbol);
        
        t_emitter.GetBlockBuilder().Builder.CreateStore(
            value,
            allocaInst
        );

        return { allocaInst, temporaries };
    }

    auto SizeOf::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->Natives->Int.GetSymbol(),
            ValueKind::R
        };
    }
}
