#include "Syntaxes/TypeParamSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TypeParamTypeSymbol.hpp"

namespace Ace
{
    TypeParamSyntax::TypeParamSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const size_t index
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_Index{ index }
    {
    }

    auto TypeParamSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto TypeParamSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto TypeParamSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto TypeParamSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto TypeParamSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::BeforeType;
    }

    auto TypeParamSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<TypeParamTypeSymbol>(
                GetSymbolScope(),
                GetName(),
                GetIndex()
            ),
            DiagnosticBag::Create(),
        };
    }

    auto TypeParamSyntax::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto TypeParamSyntax::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
