#include "Symbols/Types/VoidTypeSymbol.hpp"

#include <memory>

#include "Keyword.hpp"

namespace Ace
{
    VoidTypeSymbol::VoidTypeSymbol(
        const std::shared_ptr<Scope>& scope
    ) : m_SelfScope{ scope->GetOrCreateChild({}) },
        m_Name
        {
            SrcLocation{},
            std::string{ TokenKindToKeywordMap.at(TokenKind::VoidKeyword) },
        }
    {
    }

    auto VoidTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto VoidTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto VoidTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto VoidTypeSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::Void;
    }

    auto VoidTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto VoidTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }
}
