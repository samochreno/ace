#pragma once

#include <memory>
#include <vector>

#include "DiagnosticBag.hpp"

namespace Ace
{
    auto LogGlobalDiagnostics(
        const DiagnosticBag& t_diagnosticBag,
        const size_t t_lastLogSize
    ) -> void;

    class GlobalDiagnosticBag
    {
    public:
        template<typename T>
        auto Add(T&& t_value) -> GlobalDiagnosticBag&
        {
            m_DiagnosticBag.Add(t_value);
            LogGlobalDiagnostics(m_DiagnosticBag, m_LastLogSize);
            m_LastLogSize = m_DiagnosticBag.GetDiagnostics().size();
            return *this;
        }

    private:
        DiagnosticBag m_DiagnosticBag{};
        size_t m_LastLogSize{};
    };
}
