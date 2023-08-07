#pragma once

#include <memory>
#include <vector>

#include "DiagnosticBag.hpp"

namespace Ace
{
    auto LogGlobalDiagnostics(
        const DiagnosticBag& diagnostics,
        const size_t lastLogSize
    ) -> void;

    class GlobalDiagnosticBag
    {
    public:
        template<typename T>
        auto Add(T&& value) -> GlobalDiagnosticBag&
        {
            m_DiagnosticBag.Add(value);
            LogGlobalDiagnostics(m_DiagnosticBag, m_LastLogSize);
            m_LastLogSize = m_DiagnosticBag.GetDiagnosticGroups().size();
            return *this;
        }
        template<typename T>
        auto Collect(T value) -> void requires IsDiagnosedVoid<T> || IsExpectedVoid<T>
        {
            m_DiagnosticBag.Collect(value);
            LogGlobalDiagnostics(m_DiagnosticBag, m_LastLogSize);
            m_LastLogSize = m_DiagnosticBag.GetDiagnosticGroups().size();
        }
        template<typename T>
        auto Collect(T value)
        {
            auto result = m_DiagnosticBag.Collect(value);
            LogGlobalDiagnostics(m_DiagnosticBag, m_LastLogSize);
            m_LastLogSize = m_DiagnosticBag.GetDiagnosticGroups().size();
            return result;
        }

        auto Unwrap() const -> const DiagnosticBag&
        {
            return m_DiagnosticBag;
        }



    private:
        DiagnosticBag m_DiagnosticBag{};
        size_t m_LastLogSize{};
    };
}
