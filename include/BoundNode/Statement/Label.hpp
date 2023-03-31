#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"
#include "Scope.hpp"
#include "Symbol/Label.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Statement
{
    class Label :
        public std::enable_shared_from_this<BoundNode::Statement::Label>,
        public virtual BoundNode::Statement::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Label, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Label>
    {
    public:
        Label(Symbol::Label* const t_symbol) 
            : m_Symbol{ t_symbol }
        {
        }
        virtual ~Label() = default;

        auto GetScope() const -> Scope* final { return m_Symbol->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Label>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Label>>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetLabelSymbol() const -> Symbol::Label* { return m_Symbol; }

    private:
        Symbol::Label* m_Symbol{};
    };
}
