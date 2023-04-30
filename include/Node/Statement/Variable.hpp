#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Node/Statement/Base.hpp"
#include "Node/Typed.hpp"
#include "Node/Expression/Base.hpp"
#include "BoundNode/Statement/Variable.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Error.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Statement
{
    class Variable :
        public virtual Node::Statement::IBase,
        public virtual Node::ITyped,
        public virtual Node::ICloneable<Node::Statement::Variable>,
        public virtual Node::IBindable<BoundNode::Statement::Variable>
    {
    public:
        Variable(
            Scope* const t_scope,
            const std::string& t_name,
            const TypeName& t_typeName,
            const std::optional<std::shared_ptr<const Node::Expression::IBase>>& t_optAssignedExpression
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_TypeName{ t_typeName },
            m_OptAssignedExpression{ t_optAssignedExpression }
        {
        }
        virtual ~Variable() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::Variable> final;
        auto CloneInScopeStatement(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Variable>> final;
        auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> final { return CreateBound(); }

        auto GetName() const -> const std::string & final { return m_Name; }

        auto GetSymbolScope() const -> Scope* final { return m_Scope; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::LocalVariable; }
        auto GetSymbolCreationSuborder() const -> size_t final { return 0; }
        auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> final;

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        TypeName m_TypeName{};
        std::optional<std::shared_ptr<const Node::Expression::IBase>> m_OptAssignedExpression{};
    };
}
