#include "Symbol/Type/Struct.hpp"

namespace Ace::Symbol::Type
{
    auto Struct::CanResolveSize() const -> bool 
    {
        if (m_DidResolveSize)
            return true;

        const bool value = [&]()
        {
            if (m_IsNativeSized)
                return true;

            // If size resolution is already in progress, we have detected a circular dependency.
            if (m_IsResolvingSize)
                return false;

            m_IsResolvingSize = true;

            const auto members = GetVariables();

            const bool canResolveSize = std::find_if_not(begin(members), end(members), []
            (const Symbol::Variable::Normal::Instance* const t_member)
            {
                return t_member->GetType()->CanResolveSize();
            }) == members.end();

            if (canResolveSize)
            {
                m_IsResolvingSize = false;
                return true;
            }

            m_IsResolvingSize = false;
            return false;
        }();

        if (value)
        {
            m_DidResolveSize = true;
        }

        return value;
    }

    auto Struct::GetVariables() const -> std::vector<Symbol::Variable::Normal::Instance*>
    {
        return m_SelfScope->CollectDefinedSymbols<Symbol::Variable::Normal::Instance>();
    }
}
