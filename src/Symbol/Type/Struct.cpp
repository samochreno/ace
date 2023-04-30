#include "Symbol/Type/Struct.hpp"

#include <optional>

#include "Symbol/Function.hpp"
#include "Emittable.hpp"
#include "Core.hpp"

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

            const bool canResolveSize = std::find_if_not(begin(members), end(members),
            [](const Symbol::Variable::Normal::Instance* const t_member)
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

    auto Struct::CreateCopyGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>> 
    {
        return Core::CreateCopyGlueBody(
            GetCompilation(),
            this,
            t_glueSymbol
        );
    }

    auto Struct::CreateDropGlueBody(
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        return Core::CreateDropGlueBody(
            GetCompilation(),
            this,
            t_glueSymbol
        );
    }

    auto Struct::BindCopyGlue(Symbol::Function* const t_symbol) -> void
    {
        ACE_ASSERT(t_symbol);
        m_OptCopyGlue = t_symbol;
    }

    auto Struct::GetCopyGlue() const -> std::optional<Symbol::Function*>
    {
        return m_OptCopyGlue;
    }

    auto Struct::BindDropGlue(Symbol::Function* const t_symbol) -> void
    {
        ACE_ASSERT(t_symbol);
        m_OptDropGlue = t_symbol;
    }

    auto Struct::GetDropGlue() const -> std::optional<Symbol::Function*>
    {
        return m_OptDropGlue;
    }

    auto Struct::GetVariables() const -> std::vector<Symbol::Variable::Normal::Instance*>
    {
        auto variables = m_SelfScope->CollectDefinedSymbols<Symbol::Variable::Normal::Instance>();
        std::sort(begin(variables), end(variables),
        [](
            const Symbol::Variable::Normal::Instance* const t_lhs,
            const Symbol::Variable::Normal::Instance* const t_rhs
        )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        return variables;
    }
}
