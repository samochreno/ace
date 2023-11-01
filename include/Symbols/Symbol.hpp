#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Noun.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "SymbolCategory.hpp"
#include "AccessModifier.hpp"
#include "SrcLocation.hpp"
#include "Name.hpp"

namespace Ace
{
    class Compilation;
    class ITypeSymbol;

    struct InstantiationContext
    {
        std::vector<ITypeSymbol*> TypeArgs{};
        std::optional<ITypeSymbol*> OptSelfType{};
    };

    class ISymbol
    {
    public:
        virtual ~ISymbol() = default;

        virtual auto CreateTypeNoun() const -> Noun = 0;
        virtual auto GetCompilation() const -> Compilation* final;
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetCategory() const -> SymbolCategory = 0;
        virtual auto GetAccessModifier() const -> AccessModifier = 0;
        virtual auto GetName() const -> const Ident& = 0;

        virtual auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> = 0;

        virtual auto GetUnaliased() const -> ISymbol* final;

        virtual auto CreateLocalSignature() const -> std::string final;
        virtual auto CreateSignature() const -> std::string final;
        virtual auto CreateFullyQualifiedName(
            const SrcLocation& srcLocation
        ) const -> SymbolName final;
        virtual auto CreateLocalDisplayName() const -> std::string final;
        virtual auto CreateDisplayName() const -> std::string final;

        virtual auto IsError() const -> bool final;

        virtual auto GetRoot() const -> ISymbol*;
    };

    auto CreateUnaliasedInstantiatedSymbol(
        const IGenericSymbol* const symbol,
        const InstantiationContext& context
    ) -> IGenericSymbol*;
    template<typename T>
    auto CreateInstantiated(
        const IGenericSymbol* const symbol,
        const InstantiationContext& context
    ) -> T*
    {
        static_assert(std::is_base_of_v<IGenericSymbol, T>);

        auto* const instantiated = dynamic_cast<T*>(
            CreateUnaliasedInstantiatedSymbol(symbol, context)
        );
        ACE_ASSERT(instantiated);

        return instantiated;
    }
}
