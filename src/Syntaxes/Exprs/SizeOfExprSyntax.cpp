#include "Syntaxes/Exprs/SizeOfExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    SizeOfExprSyntax::SizeOfExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeName& typeName
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope}, 
        m_TypeName{ typeName }
    {
    }

    auto SizeOfExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SizeOfExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SizeOfExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto SizeOfExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const SizeOfExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ITypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return Diagnosed
        {
            std::make_shared<const SizeOfExprSema>(
                GetSrcLocation(),
                GetScope(),
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto SizeOfExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
