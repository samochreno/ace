#pragma once

#include <memory>
#include <vector>

namespace Ace
{
    template<typename TDiagnostic>
    class DiagnosticBag
    {
    public:
        DiagnosticBag() = default;
        ~DiagnosticBag() = default;

        template<typename TDiagnosticNew, typename... A>
        auto Add(A&&... args) -> void
        {
            m_Diagnostics.push_back(std::make_shared<TDiagnosticNew>(
                std::forward<A>(args)...
            ));
        }
        template<typename TDiagnosticNew>
        auto Add(
            const std::shared_ptr<const TDiagnosticNew>& t_diagnostic
        ) -> void
        {
            m_Diagnostics.push_back(t_diagnostic);
        }
        template<typename TDiagnosticNew>
        auto Add(
            const DiagnosticBag<TDiagnosticNew>& t_diagnosticBag
        ) -> void
        {
            m_Diagnostics.insert(
                end(m_Diagnostics),
                begin(t_diagnosticBag.GetDiagnostics()),
                end  (t_diagnosticBag.GetDiagnostics())
            );
        }

        auto GetDiagnostics() const -> const std::vector<std::shared_ptr<const TDiagnostic>>& { return m_Diagnostics; }

    private:
        std::vector<std::shared_ptr<const TDiagnostic>> m_Diagnostics{};
    };
}
