#include "Syntaxes/Exprs/TypeInfoPtrExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/TypeInfoPtrExprSema.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"

namespace Ace
{
    TypeInfoPtrExprSyntax::TypeInfoPtrExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeName& typeName
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeName{ typeName }
    {
    }

    auto TypeInfoPtrExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto TypeInfoPtrExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto TypeInfoPtrExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto TypeInfoPtrExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const TypeInfoPtrExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ISizedTypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed
        {
            std::make_shared<const TypeInfoPtrExprSema>(
                GetSrcLocation(),
                GetScope(),
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto TypeInfoPtrExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
