#include "BoundNodes/Exprs/LiteralExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "ValueKind.hpp"
#include "Assert.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "String.hpp"

namespace Ace
{
    LiteralExprBoundNode::LiteralExprBoundNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const LiteralKind t_kind,
        const std::string& t_string
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Kind{ t_kind },
        m_String{ t_string }
    {
    }

    auto LiteralExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto LiteralExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LiteralExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto LiteralExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const LiteralExprBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto LiteralExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto LiteralExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const LiteralExprBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto LiteralExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto LiteralExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        auto* const type = t_emitter.GetIRType(GetTypeInfo().Symbol);

        auto* const value = [&]() -> llvm::Value*
        {
            if (m_Kind & LiteralKind::NumberMask)
            {
                std::string numberString{};
                (void)std::find_if_not(begin(m_String), end(m_String),
                [&](const char t_character)
                {
                    if (IsInAlphabet(t_character))
                    {
                        return false;
                    }

                    if (IsNumber(t_character) || (t_character == '.'))
                    {
                        numberString += t_character;
                    }

                    return true;
                });

                if (m_Kind & LiteralKind::IntMask)
                {
                    const uint64_t value = std::stoull(numberString);
                    return llvm::ConstantInt::get(
                        type,
                        value
                    );
                }
                else if (m_Kind & LiteralKind::FloatMask)
                {
                    const double value = std::stod(numberString);
                    return llvm::ConstantFP::get(
                        type,
                        value
                    );
                }

                ACE_UNREACHABLE();
            }
            else if (m_Kind == LiteralKind::String)
            {
                // TODO: Finish strings.
                return nullptr;
            }
            else if (m_Kind & LiteralKind::BoolMask)
            {
                return llvm::ConstantInt::get(
                    type,
                    (m_Kind == LiteralKind::True) ? 1 : 0
                );
            }

            ACE_UNREACHABLE();
        }();

        ACE_ASSERT(value);
        
        auto* const allocaInst =
            t_emitter.GetBlockBuilder().Builder.CreateAlloca(value->getType());
        t_emitter.GetBlockBuilder().Builder.CreateStore(value, allocaInst);

        return { allocaInst, { { allocaInst, GetTypeInfo().Symbol } } };
    }

    auto LiteralExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        auto& natives = GetCompilation()->Natives;

        auto* const typeSymbol = [&]() -> ITypeSymbol*
        {
            if      (m_Kind == LiteralKind::Int8)    return natives->Int8.GetSymbol();
            else if (m_Kind == LiteralKind::Int16)   return natives->Int16.GetSymbol();
            else if (m_Kind == LiteralKind::Int32)   return natives->Int32.GetSymbol();
            else if (m_Kind == LiteralKind::Int64)   return natives->Int64.GetSymbol();

            else if (m_Kind == LiteralKind::UInt8)   return natives->UInt8.GetSymbol();
            else if (m_Kind == LiteralKind::UInt16)  return natives->UInt16.GetSymbol();
            else if (m_Kind == LiteralKind::UInt32)  return natives->UInt32.GetSymbol();
            else if (m_Kind == LiteralKind::UInt64)  return natives->UInt64.GetSymbol();

            else if (m_Kind == LiteralKind::Int)     return natives->Int.GetSymbol();

            else if (m_Kind == LiteralKind::Float32) return natives->Float32.GetSymbol();
            else if (m_Kind == LiteralKind::Float64) return natives->Float64.GetSymbol();
            
            else if (m_Kind == LiteralKind::String)  return natives->String.GetSymbol();
            else if (m_Kind & LiteralKind::BoolMask) return natives->Bool.GetSymbol();

            ACE_UNREACHABLE();
        }();

        return { typeSymbol, ValueKind::R };
    }
}
