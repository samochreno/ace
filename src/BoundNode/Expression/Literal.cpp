#include "BoundNode/Expression/Literal.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "Symbol/Type/Base.hpp"
#include "ValueKind.hpp"
#include "Asserts.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "Utility.hpp"
#include "NativeSymbol.hpp"

namespace Ace::BoundNode::Expression
{
    auto Literal::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Literal::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Literal>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Literal::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Literal>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Literal::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        auto* const type = t_emitter.GetIRType(GetTypeInfo().Symbol);

        auto* const value = [&]() -> llvm::Value*
        {
            if (m_Kind & LiteralKind::NumberMask)
            {
                std::string numberString{};
                (void)std::find_if_not(begin(m_String), end(m_String), [&]
                (const char& t_char)
                {
                    if (IsInAlphabet(t_char))
                        return false;

                    if (IsNumber(t_char) || (t_char == '.'))
                        numberString += t_char;

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
                    const double_t value = std::stod(numberString);
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
        
        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(value->getType());
        t_emitter.GetBlockBuilder().Builder.CreateStore(value, allocaInst);

        return { allocaInst, { { allocaInst, GetTypeInfo().Symbol } } };
    }

    auto Literal::GetTypeInfo() const -> TypeInfo
    {
        auto* const typeSymbol = [&]() -> Symbol::Type::IBase*
        {
            if      (m_Kind == LiteralKind::Int8)       return NativeSymbol::Int8.GetSymbol();
            else if (m_Kind == LiteralKind::Int16)      return NativeSymbol::Int16.GetSymbol();
            else if (m_Kind == LiteralKind::Int32)      return NativeSymbol::Int32.GetSymbol();
            else if (m_Kind == LiteralKind::Int64)      return NativeSymbol::Int64.GetSymbol();

            else if (m_Kind == LiteralKind::UInt8)      return NativeSymbol::UInt8.GetSymbol();
            else if (m_Kind == LiteralKind::UInt16)     return NativeSymbol::UInt16.GetSymbol();
            else if (m_Kind == LiteralKind::UInt32)     return NativeSymbol::UInt32.GetSymbol();
            else if (m_Kind == LiteralKind::UInt64)     return NativeSymbol::UInt64.GetSymbol();

            else if (m_Kind == LiteralKind::Int)        return NativeSymbol::Int.GetSymbol();

            else if (m_Kind == LiteralKind::Float32)    return NativeSymbol::Float32.GetSymbol();
            else if (m_Kind == LiteralKind::Float64)    return NativeSymbol::Float64.GetSymbol();
            
            else if (m_Kind == LiteralKind::String)     return NativeSymbol::String.GetSymbol();
            else if (m_Kind & LiteralKind::BoolMask)    return NativeSymbol::Bool.GetSymbol();

            ACE_UNREACHABLE();
        }();

        return { typeSymbol, ValueKind::R };
    }
}
