#include "Node/Expression/SizeOf.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbol/Type/Base.hpp"

namespace Ace::Node::Expression
{
    auto SizeOf::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto SizeOf::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::SizeOf>
    {
        return std::make_shared<const Node::Expression::SizeOf>(
            t_scope,
            m_TypeName
        );
    }

    auto SizeOf::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::SizeOf>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::make_shared<const BoundNode::Expression::SizeOf>(
            m_Scope,
            typeSymbol
        );
    }
}
