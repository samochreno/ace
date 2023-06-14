#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "DiagnosticsBase.hpp"

namespace Ace
{
    template<typename TValue, typename TDiagnostic = IDiagnostic>
    class Diagnosed;

    template<typename TDiagnostic>
    class Diagnosed<void, TDiagnostic>
    {
        static_assert(
            std::is_base_of_v<IDiagnostic, TDiagnostic>,
            "TDiagnostic must be derived from IDiagnostic"
        );

    public:
        Diagnosed() = default;
        Diagnosed(
            const std::vector<std::shared_ptr<const TDiagnostic>>& t_diagnostics
        ) : m_Diagnostics{ t_diagnostics }
        {
        }

        auto GetDiagnostics() const -> const std::vector<std::shared_ptr<const TDiagnostic>>&
        {
            return m_Diagnostics;
        }

    private:
        std::vector<std::shared_ptr<const TDiagnostic>> m_Diagnostics{};
    };

    template<typename TValue, typename TDiagnostic>
    class Diagnosed
    {
        static_assert(
            std::is_base_of_v<IDiagnostic, TDiagnostic>,
            "TDiagnostic must be derived from IDiagnostic"
        );

    public:
        Diagnosed(
            const TValue& t_value
        ) : m_Value{ t_value }
        {
        }
        Diagnosed(
            const TValue& t_value,
            const std::vector<std::shared_ptr<const TDiagnostic>>& t_diagnostics
        ) : m_Value{ t_value },
            m_Diagnostics{ t_diagnostics }
        {
        }

        auto Unwrap() -> TValue&
        {
            return m_Value;
        }
        auto Unwrap() const -> const TValue&
        {
            return m_Value;
        }

        auto GetDiagnostics() const -> const std::vector<std::shared_ptr<const TDiagnostic>>&
        {
            return m_Diagnostics;
        }

    private:
        TValue m_Value{};
        std::vector<std::shared_ptr<const TDiagnostic>> m_Diagnostics{};
    };
}
