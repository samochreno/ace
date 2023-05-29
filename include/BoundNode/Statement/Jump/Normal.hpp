#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Jump/Base.hpp"
#include "Scope.hpp"
#include "Symbol/Label.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Statement::Jump
{
    class Normal :
        public std::enable_shared_from_this<BoundNode::Statement::Jump::Normal>,
        public virtual BoundNode::Statement::Jump::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Statement::Jump::Normal, BoundNode::Statement::Context::TypeChecking>,
        public virtual BoundNode::ILowerable<BoundNode::Statement::Jump::Normal>
    {
    public:
        Normal(
            const std::shared_ptr<Scope>& t_scope,
            Symbol::Label* const t_labelSymbol
        ) : m_Scope{ t_scope },
            m_LabelSymbol{ t_labelSymbol }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Jump::Normal>>> final;
        auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Jump::Normal>> final;
        auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetLabelSymbol() const -> Symbol::Label* final { return m_LabelSymbol; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        Symbol::Label* m_LabelSymbol{};
    };
}
