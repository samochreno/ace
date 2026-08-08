#include "Syntaxes/Syntax.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Compilation.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
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
    ) -> Diagnosed<std::vector<ITypeSymbol*>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<ITypeSymbol*> symbols{};
        std::unordered_map<std::string, const TypeParamSyntax*> firstTypeParams{};
        std::unordered_map<std::string, ITypeSymbol*> firstTypeParamSymbols{};
        std::for_each(begin(typeParams), end(typeParams),
        [&](const std::shared_ptr<const TypeParamSyntax>& typeParam)
        {
            const auto& name = typeParam->GetName();

            const auto [it, inserted] = firstTypeParams.emplace(
                name.String,
                typeParam.get()
            );
            if (!inserted)
            {
                diagnostics.Add(CreateTypeParamRedeclarationError(
                    it->second->GetSrcLocation(),
                    typeParam->GetSrcLocation()
                ));
                symbols.push_back(firstTypeParamSymbols.at(name.String));
                return;
            }

            const auto optSymbol = diagnostics.Collect(
                scope->ResolveStaticSymbol<TypeParamTypeSymbol>(name)
            );
            auto* const symbol = optSymbol.has_value() ?
                static_cast<ITypeSymbol*>(optSymbol.value()) :
                scope->GetCompilation()->GetErrorSymbols().GetType();
            firstTypeParamSymbols[name.String] = symbol;
            symbols.push_back(symbol);
        });

        return Diagnosed{ symbols, std::move(diagnostics) };
    }
}
