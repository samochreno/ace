#include "Syntaxes/Syntax.hpp"

#include <memory>
#include <vector>

#include "Compilation.hpp"
#include "Scope.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TypeParamTypeSymbol.hpp"

namespace Ace
{
    auto SyntaxChildCollector::Build() const -> std::vector<const ISyntax*>
    {
        return std::move(m_Children);
    }

    auto ISyntax::GetCompilation() const -> Compilation*
    {
        return GetScope()->GetCompilation();
    }

    auto ResolveTypeParamSymbols(
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams
    ) -> std::vector<ITypeSymbol*>
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        std::vector<ITypeSymbol*> symbols{};
        std::transform(
            begin(typeParams),
            end  (typeParams),
            back_inserter(symbols),
            [&](const std::shared_ptr<const TypeParamSyntax>& typeParam)
            {
                const auto& name = typeParam->GetName();
                return diagnostics.Collect(
                    scope->ResolveStaticSymbol<TypeParamTypeSymbol>(name)
                ).value();
            }
        );

        return symbols;
    }
}
