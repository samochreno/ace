#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNode/Statement/Base.hpp"
#include "BoundNode/Typed.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Symbol/Variable/Local.hpp"

namespace Ace::BoundNode::Statement
{
    class Variable :
        public std::enable_shared_from_this<BoundNode::Statement::Variable>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::ITyped<Symbol::Variable::Local>,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Variable, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Variable>
    {
    public:
        Variable(
            Symbol::Variable::Local* const t_symbol,
            const std::optional<std::shared_ptr<const BoundNode::Expression::IBase>>& t_optAssignedExpression
        ) : m_Symbol{ t_symbol },
            m_OptAssignedExpression{ t_optAssignedExpression }
        {
        }
        virtual ~Variable() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Symbol->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Variable>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Variable>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetSymbol() const -> Symbol::Variable::Local* final { return m_Symbol; }
        
    private:
        Symbol::Variable::Local* m_Symbol{};
        std::optional<std::shared_ptr<const BoundNode::Expression::IBase>> m_OptAssignedExpression{};
    };
}
