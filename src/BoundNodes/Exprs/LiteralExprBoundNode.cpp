#include "BoundNodes/Exprs/LiteralExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "String.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    LiteralExprBoundNode::LiteralExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const LiteralKind kind,
        const std::string& string
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Kind{ kind },
        m_String{ string }
    {
    }
    
    auto LiteralExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LiteralExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LiteralExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto LiteralExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const LiteralExprBoundNode>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto LiteralExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto LiteralExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const LiteralExprBoundNode>
    {
        return shared_from_this();
    }

    auto LiteralExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto LiteralExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        auto* const type = emitter.GetIRType(GetTypeInfo().Symbol);

        auto* const value = [&]() -> llvm::Value*
        {
            if (m_Kind & LiteralKind::NumberMask)
            {
                std::string numberString{};
                (void)std::find_if_not(begin(m_String), end(m_String),
                [&](const char character)
                {
                    if (IsInAlphabet(character))
                    {
                        return false;
                    }

                    if (IsNumber(character) || (character == '.'))
                    {
                        numberString += character;
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
                // TODO: Finish strings
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
            emitter.GetBlockBuilder().Builder.CreateAlloca(value->getType());
        emitter.GetBlockBuilder().Builder.CreateStore(value, allocaInst);

        return { allocaInst, { { allocaInst, GetTypeInfo().Symbol } } };
    }

    auto LiteralExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        const auto& natives = GetCompilation()->GetNatives();

        auto* const typeSymbol = [&]() -> ITypeSymbol*
        {
            if      (m_Kind == LiteralKind::Int8)    return natives.Int8.GetSymbol();
            else if (m_Kind == LiteralKind::Int16)   return natives.Int16.GetSymbol();
            else if (m_Kind == LiteralKind::Int32)   return natives.Int32.GetSymbol();
            else if (m_Kind == LiteralKind::Int64)   return natives.Int64.GetSymbol();

            else if (m_Kind == LiteralKind::UInt8)   return natives.UInt8.GetSymbol();
            else if (m_Kind == LiteralKind::UInt16)  return natives.UInt16.GetSymbol();
            else if (m_Kind == LiteralKind::UInt32)  return natives.UInt32.GetSymbol();
            else if (m_Kind == LiteralKind::UInt64)  return natives.UInt64.GetSymbol();

            else if (m_Kind == LiteralKind::Int)     return natives.Int.GetSymbol();

            else if (m_Kind == LiteralKind::Float32) return natives.Float32.GetSymbol();
            else if (m_Kind == LiteralKind::Float64) return natives.Float64.GetSymbol();
            
            else if (m_Kind == LiteralKind::String)  return natives.String.GetSymbol();
            else if (m_Kind & LiteralKind::BoolMask) return natives.Bool.GetSymbol();

            ACE_UNREACHABLE();
        }();

        return { typeSymbol, ValueKind::R };
    }
}
