#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expr/Base.hpp"
#include "Symbols/Vars/VarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace::BoundNode::Expr::VarReference
{
    class Static :
        public std::enable_shared_from_this<BoundNode::Expr::VarReference::Static>,
        public virtual BoundNode::Expr::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expr::VarReference::Static>,
        public virtual BoundNode::ILowerable<BoundNode::Expr::VarReference::Static>
    {
    public:
        Static(
            const std::shared_ptr<Scope>& t_scope,
            IVarSymbol* const t_variableSymbol
        ) : m_Scope{ t_scope },
            m_VarSymbol{ t_variableSymbol }
        {
        }
        virtual ~Static() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::VarReference::Static>>> final;
        auto GetOrCreateTypeCheckedExpr(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::VarReference::Static>> final;
        auto GetOrCreateLoweredExpr(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetVarSymbol() const -> IVarSymbol* { return m_VarSymbol; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        IVarSymbol* m_VarSymbol{};
    };
}
