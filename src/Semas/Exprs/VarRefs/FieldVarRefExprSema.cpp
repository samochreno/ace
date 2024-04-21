#include "Semas/Exprs/VarRefs/FieldVarRefExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Symbols/Vars/FieldVarSymbol.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    FieldVarRefExprSema::FieldVarRefExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr,
        FieldVarSymbol* const fieldSymbol 
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_FieldSymbol{ fieldSymbol }
    {
    }

    auto FieldVarRefExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("FieldVarRefExprSema", [&]()
        {
            logger.Log("m_Expr", m_Expr);
            logger.Log("m_FieldSymbol", m_FieldSymbol);
        });
    }

    auto FieldVarRefExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FieldVarRefExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto FieldVarRefExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const FieldVarRefExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto checkedExpr = diagnostics.Collect(
            m_Expr->CreateTypeCheckedExpr({})
        );

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const FieldVarRefExprSema>(
                GetSrcLocation(),
                checkedExpr,
                m_FieldSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto FieldVarRefExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto FieldVarRefExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const FieldVarRefExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const FieldVarRefExprSema>(
            GetSrcLocation(),
            loweredExpr,
            m_FieldSymbol
        )->CreateLowered({});
    }

    auto FieldVarRefExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    static auto CreateDerefed(
        std::shared_ptr<const IExprSema> expr,
        FieldVarSymbol* const fieldSymbol
    ) -> std::shared_ptr<const IExprSema>
    {
        while (true)
        {
            auto* const typeSymbol = expr->GetTypeInfo().Symbol;
            if (typeSymbol->GetUnaliased() == fieldSymbol->GetParentStruct())
            {
                break;
            }

            if (typeSymbol->IsRef())
            {
                expr = std::make_shared<const DerefAsExprSema>(
                    expr->GetSrcLocation(),
                    expr,
                    typeSymbol->GetWithoutRef()
                );
            }
            else if (typeSymbol->IsAnyStrongPtr())
            {
                expr = std::make_shared<const DerefAsExprSema>(
                    expr->GetSrcLocation(),
                    expr,
                    typeSymbol->GetWithoutStrongPtr()
                );
            }
            else
            {
                ACE_UNREACHABLE();
            }
        }

        return expr;
    }

    auto FieldVarRefExprSema::CollectMonos() const -> MonoCollector 
    {
        return MonoCollector{}
            .Collect(m_Expr)
            .Collect(m_FieldSymbol);
    }

    auto FieldVarRefExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        auto* const fieldSymbol = dynamic_cast<FieldVarSymbol*>(m_FieldSymbol);
        ACE_ASSERT(fieldSymbol);

        const auto expr = CreateDerefed(m_Expr, m_FieldSymbol);
        const auto exprEmitResult = expr->Emit(emitter);
        tmps.insert(
            end  (tmps), 
            begin(exprEmitResult.Tmps), 
            end  (exprEmitResult.Tmps)
        );

        auto* const exprTypeSymbol = expr->GetTypeInfo().Symbol;
        auto* const exprType = emitter.GetType(exprTypeSymbol);

        const auto index = fieldSymbol->GetIndex();

        auto* const int32Type = llvm::Type::getInt32Ty(emitter.GetContext());

        auto* const gepInst = emitter.GetBlock().Builder.CreateStructGEP(
            exprType,
            exprEmitResult.Value,
            index
        );

        return { gepInst, tmps };
    }

    auto FieldVarRefExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { m_FieldSymbol->GetType(), ValueKind::L };
    }

    auto FieldVarRefExprSema::GetExpr() const -> std::shared_ptr<const IExprSema>
    {
        return m_Expr;
    }

    auto FieldVarRefExprSema::GetFieldSymbol() const -> FieldVarSymbol*
    {
        return m_FieldSymbol;
    }
}
