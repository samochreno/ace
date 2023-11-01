#include "Symbols/Types/VoidTypeSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Keyword.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    VoidTypeSymbol::VoidTypeSymbol(
        const std::shared_ptr<Scope>& scope
    ) : m_BodyScope{ scope->CreateChild() },
        m_Name
        {
            SrcLocation{ GetCompilation() },
            std::string{ TokenKindToKeywordMap.at(TokenKind::VoidKeyword) },
        }
    {
    }

    auto VoidTypeSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::The, "void type" };
    }

    auto VoidTypeSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto VoidTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto VoidTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Pub;
    }

    auto VoidTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto VoidTypeSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        ACE_UNREACHABLE();
    }

    auto VoidTypeSymbol::SetBodyScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        ACE_UNREACHABLE();
    }

    auto VoidTypeSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        static const std::vector<ITypeSymbol*> info{};
        return info;
    }
}
