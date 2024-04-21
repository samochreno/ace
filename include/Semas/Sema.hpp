#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <algorithm>
#include <iterator>

#include "Compilation.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    class SemaLogger;

    struct TypeCheckingContext
    {
    };

    struct LoweringContext
    {
    };

    class MonoCollector
    {
    public:
        MonoCollector() = default;
        ~MonoCollector() = default;

        template<typename T>
        auto Collect(T* const symbol) -> MonoCollector&
        {
            static_assert(
                std::is_base_of_v<IGenericSymbol, T> ||
                std::is_base_of_v<ITypedSymbol, T>
            );

            if constexpr (std::is_base_of_v<IGenericSymbol, T>)
            {
                if (symbol->IsPlaceholder())
                {
                    m_Placeholders.push_back(symbol);
                }
            }

            if constexpr (std::is_base_of_v<ITypedSymbol, T>)
            {
                if (symbol->GetType()->IsPlaceholder())
                {
                    m_Placeholders.push_back(symbol->GetType());
                }
            }

            return *this;
        }
        template<typename T>
        auto Collect(const std::shared_ptr<const T>& sema) -> MonoCollector&
        {
            const auto collector = sema->CollectMonos();
            const auto& placeholders = collector.Get();
            m_Placeholders.insert(
                end(m_Placeholders),
                begin(placeholders),
                end  (placeholders)
            );

            return *this;
        }
        template<typename T>
        auto Collect(
            const std::optional<std::shared_ptr<const T>>& optSema
        ) -> MonoCollector&
        {
            if (optSema.has_value())
            {
                Collect(optSema.value());
            }

            return *this;
        }
        template<typename T>
        auto Collect(
            const std::vector<std::shared_ptr<const T>>& semas
        ) -> MonoCollector&
        {
            std::for_each(begin(semas), end(semas),
            [&](const std::shared_ptr<const T>& sema)
            {
                Collect(sema);
            });

            return *this;
        }

        auto Get() const -> const std::vector<IGenericSymbol*>&;

    private:
        std::vector<IGenericSymbol*> m_Placeholders{};
    };

    class ISema
    {
    public:
        virtual ~ISema() = default;

        virtual auto Log(SemaLogger& logger) const -> void = 0;

        virtual auto GetCompilation() const -> Compilation* final;
        virtual auto GetSrcLocation() const -> const SrcLocation& = 0;
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;

        virtual auto CollectMonos() const -> MonoCollector = 0;
    };

    template<typename T, typename TContext = TypeCheckingContext>
    class ITypeCheckableSema : public virtual ISema
    {
    public:
        virtual ~ITypeCheckableSema() = default;

        virtual auto CreateTypeChecked(
            const TContext& context
        ) const -> Diagnosed<std::shared_ptr<const T>> = 0;
    };

    template<typename T, typename TContext = LoweringContext>
    class ILowerableSema : public virtual ISema
    {
    public:
        virtual ~ILowerableSema() = default;

        virtual auto CreateLowered(
            const TContext& context
        ) const -> std::shared_ptr<const T> = 0;
    };
}
