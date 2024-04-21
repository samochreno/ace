#include "Semas/Exprs/AddressOfExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropInfo.hpp"

namespace Ace
{
    AddressOfExprSema::AddressOfExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto AddressOfExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("AddressOfExprSema", [&]()
        {
            logger.Log("m_Expr", m_Expr);
        });
    }

    auto AddressOfExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AddressOfExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto AddressOfExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const AddressOfExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }
         
        return Diagnosed
        {
            std::make_shared<const AddressOfExprSema>(
                GetSrcLocation(),
                checkedExpr
            ),
            std::move(diagnostics),
        };
    }

    auto AddressOfExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto AddressOfExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const AddressOfExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const AddressOfExprSema>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto AddressOfExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto AddressOfExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Expr);
    }

    auto AddressOfExprSema::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );

        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        auto* const type = llvm::PointerType::get(
            emitter.GetType(typeSymbol),
            0
        );

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(type);
        tmps.emplace_back(
            allocaInst, 
            GetCompilation()->GetNatives().Ptr.GetSymbol()
        );

        emitter.GetBlock().Builder.CreateStore(
            exprEmitResult.Value, 
            allocaInst
        );

        return { allocaInst, tmps };
    }

    auto AddressOfExprSema::GetTypeInfo() const -> TypeInfo
    {
        return 
        { 
            GetCompilation()->GetNatives().Ptr.GetSymbol(), 
            ValueKind::R
        };
    }
}
