#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Statement/Group.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Symbol/Function.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace::BoundNode::Statement::Assignment
{
    class Compound :
        public std::enable_shared_from_this<BoundNode::Statement::Assignment::Compound>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Assignment::Compound, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Group>
    {
    public:
        Compound(
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_lhsExpression,
            const std::shared_ptr<const BoundNode::Expression::IBase>& t_rhsExpression,
            Symbol::Function* const t_operatorSymbol
        ) : m_LHSExpression{ t_lhsExpression },
            m_RHSExpression{ t_rhsExpression },
            m_OperatorSymbol{ t_operatorSymbol }
        {
        }
        virtual ~Compound() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_LHSExpression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Assignment::Compound>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Group>>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final { ACE_UNREACHABLE(); }

    private:
        std::shared_ptr<const BoundNode::Expression::IBase> m_LHSExpression{};
        std::shared_ptr<const BoundNode::Expression::IBase> m_RHSExpression{};
        Symbol::Function* m_OperatorSymbol{};
    };
}
