#include "Semas/Exprs/LiteralExprSema.hpp"

#include <memory>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "SemaLogger.hpp"
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
    LiteralExprSema::LiteralExprSema(
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

    auto LiteralExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("LiteralExprSema", [&]()
        {
            logger.Log("m_String", m_String);
        });
    }
    
    auto LiteralExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LiteralExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LiteralExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const LiteralExprSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto LiteralExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto LiteralExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const LiteralExprSema>
    {
        return shared_from_this();
    }

    auto LiteralExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto LiteralExprSema::CollectMonos() const -> MonoCollector
    {
        return {};
    }

    auto LiteralExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        auto* const type = emitter.GetType(GetTypeInfo().Symbol);

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
            emitter.GetBlock().Builder.CreateAlloca(value->getType());
        emitter.GetBlock().Builder.CreateStore(value, allocaInst);

        return { allocaInst, { { allocaInst, GetTypeInfo().Symbol } } };
    }

    auto LiteralExprSema::GetTypeInfo() const -> TypeInfo
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
