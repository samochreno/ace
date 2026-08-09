#include <array>
#include <optional>

#include "Assert.hpp"
#include "Diagnostics/CompilationDiagnostics.hpp"
#include "Natives.hpp"

namespace
{
    class MissingNative final : public Ace::INative
    {
    public:
        auto GetCompilation() const -> Ace::Compilation* final
        {
            return nullptr;
        }

        auto CreateFullyQualifiedName(const Ace::SrcLocation& srcLocation) const
            -> Ace::SymbolName final
        {
            return Ace::SymbolName{
                {
                    Ace::SymbolNameSection{ Ace::Ident{ srcLocation, "std" } },
                    Ace::SymbolNameSection{ Ace::Ident{ srcLocation, "missing" } },
                },
                Ace::SymbolNameResolutionScope::Global,
            };
        }

        auto TryGetGenericSymbol() const -> std::optional<Ace::ISymbol*> final
        {
            return std::nullopt;
        }
    };
}

auto main() -> int
{
    using namespace Ace;

    MissingNative missingNative{};
    const std::array<INative*, 1> natives{ &missingNative };

    const auto verification = VerifyNativeSymbols(natives);
    ACE_ASSERT(verification.GetDiagnostics().HasErrors());

    const auto diagnostic = CreateMissingNativeSymbolError(&missingNative);
    ACE_ASSERT(diagnostic.Diagnostics.size() == 1);
    ACE_ASSERT(diagnostic.Diagnostics.front().Severity == DiagnosticSeverity::Error);
    ACE_ASSERT(!diagnostic.Diagnostics.front().OptSrcLocation.has_value());
    ACE_ASSERT(diagnostic.Diagnostics.front().Message == "missing native symbol `std::missing`");
}
