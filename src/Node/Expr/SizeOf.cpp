#include "Node/Expr/SizeOf.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace::Node::Expr
{
    auto SizeOf::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto SizeOf::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::SizeOf>
    {
        return std::make_shared<const Node::Expr::SizeOf>(
            t_scope,
            m_TypeName
        );
    }

    auto SizeOf::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::SizeOf>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::make_shared<const BoundNode::Expr::SizeOf>(
            m_Scope,
            typeSymbol
        );
    }
}
