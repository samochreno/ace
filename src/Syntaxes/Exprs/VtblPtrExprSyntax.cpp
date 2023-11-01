#include "Syntaxes/Exprs/VtblPtrExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/VtblPtrExprSema.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"

namespace Ace
{
    VtblPtrExprSyntax::VtblPtrExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeName& typeName,
        const TypeName& traitName
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeName{ typeName },
        m_TraitName{ traitName }
    {
    }

    auto VtblPtrExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto VtblPtrExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto VtblPtrExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto VtblPtrExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const VtblPtrExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ISizedTypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        const auto optTraitSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ITypeSymbol>(GetScope(), m_TraitName)
        );
        auto* const traitSymbol = optTraitSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetTrait()
        );

        return Diagnosed
        {
            std::make_shared<const VtblPtrExprSema>(
                GetSrcLocation(),
                GetScope(),
                typeSymbol,
                traitSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto VtblPtrExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
