#include "Symbols/Types/TypeParamTypeSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "AccessModifier.hpp"
#include "Assert.hpp"
#include "Symbols/TypeParamOwnerSymbol.hpp"

namespace Ace
{
    TypeParamTypeSymbol::TypeParamTypeSymbol(
        const std::shared_ptr<Scope>& scope, const Ident& name, const size_t index
    )
        : m_BodyScope{ scope->CreateChild() },
          m_Name{ name },
          m_Parent{},
          m_Index{ index }
    {
    }

    auto TypeParamTypeSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "type parameter" };
    }

    auto TypeParamTypeSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto TypeParamTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto TypeParamTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Priv;
    }

    auto TypeParamTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto TypeParamTypeSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope, const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        ACE_UNREACHABLE();
    }

    auto TypeParamTypeSymbol::SetBodyScope(const std::shared_ptr<Scope>& scope) -> void
    {
        m_BodyScope = scope;
    }

    auto TypeParamTypeSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        static const std::vector<ITypeSymbol*> args{};
        return args;
    }

    auto TypeParamTypeSymbol::BindParent(ITypeParamOwnerSymbol* const parent) -> void
    {
        ACE_ASSERT(parent);
        ACE_ASSERT(!m_Parent || m_Parent == parent);
        m_Parent = parent;
    }

    auto TypeParamTypeSymbol::GetParent() const -> ITypeParamOwnerSymbol*
    {
        ACE_ASSERT(m_Parent);
        return m_Parent;
    }

    auto TypeParamTypeSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
