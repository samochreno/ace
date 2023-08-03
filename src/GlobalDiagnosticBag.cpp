#include "GlobalDiagnosticBag.hpp"

#include <memory>
#include <vector>

#include "DiagnosticBag.hpp"
#include "SrcBuffer.hpp"
#include "Log.hpp"

namespace Ace
{
    constexpr size_t MaxSnippetWidth = 80 - 3;

    struct DiagnosticSnippetDisplayInfo
    {
        std::string_view String{};
        std::string Highlight{};
    };

    static auto CreateHighlightString(
        const std::string_view string,
        const std::string_view highlight
    ) -> std::string
    {
        std::string highlightString{};

        std::transform(
            begin(string),
            begin(highlight),
            back_inserter(highlightString),
            [](const char character)
            {
                return ' ';
            }
        );

        std::transform(
            begin(highlight),
            end  (highlight),
            back_inserter(highlightString),
            [](const char character)
            {
                return '^';
            }
        );

        std::transform(
            end(highlight),
            end(string),
            back_inserter(highlightString),
            [](const char character)
            {
                return ' ';
            }
        );

        return highlightString;
    }

    static auto CalculateSnippetString(
        const std::string_view line,
        const std::string_view highlight
    ) -> std::string_view
    {
        if (line.size() <= MaxSnippetWidth)
        {
            return line;
        }

        if (highlight.size() >= MaxSnippetWidth)
        {
            return std::string_view
            {
                begin(highlight),
                end  (highlight) + MaxSnippetWidth,
            };
        }

        const auto padding      = MaxSnippetWidth - highlight.size();
        const auto paddingLeft  = padding / 2;
        const auto paddingRight = padding - paddingLeft;

        return std::string_view
        {
            begin(highlight) - paddingLeft,
            end  (highlight) + paddingRight,
        };
    }

    static auto CalculateSnippetStringLine(
        const SrcLocation& srcLocation
    ) -> std::string_view
    {
        const std::string_view bufferView = srcLocation.Buffer->GetBuffer();

        const auto bufferBeginIt = begin(bufferView);
        const auto bufferEndIt   = end  (bufferView);

        const auto lineBeginIt = std::find(
            std::make_reverse_iterator(srcLocation.CharacterBeginIterator),
            std::make_reverse_iterator(bufferBeginIt),
            '\n'
        );

        const auto lineEndIt = std::find(
            srcLocation.CharacterBeginIterator,
            bufferEndIt,
            '\n'
        );

        return std::string_view{ lineBeginIt.base(), lineEndIt };
    }

    auto CreateCappedIterator(
        const std::string_view line,
        std::string_view::const_iterator it
    ) -> std::string_view::const_iterator
    {
        const auto beginIt = begin(line);
        const auto   endIt =   end(line);

        if (it < beginIt)
        {
            it = beginIt;
        }
        else if (it > endIt)
        {
            it = endIt;
        }

        return it;
    }

    static auto CalculateSnippetDisplayInfo(
        const SrcLocation& srcLocation
    ) -> DiagnosticSnippetDisplayInfo
    {
        const auto line = CalculateSnippetStringLine(srcLocation);

        auto highlight = std::string_view
        {
            CreateCappedIterator(line, srcLocation.CharacterBeginIterator),
            CreateCappedIterator(line, srcLocation.CharacterEndIterator),
        };

        const auto string = CalculateSnippetString(
            line,
            highlight
        );

        highlight = std::string_view
        {
            CreateCappedIterator(string, begin(highlight)),
            CreateCappedIterator(string, end  (highlight)),
        };

        const auto highlightString = CreateHighlightString(
            string,
            highlight
        );

        return DiagnosticSnippetDisplayInfo{ string, highlightString };
    }

    static auto LogSeverityColor(const DiagnosticSeverity severity)
    {
        switch (severity)
        {
            case DiagnosticSeverity::Info:
            {
                break;
            }

            case DiagnosticSeverity::Note:
            {
                Log << termcolor::bright_green;
                break;
            }

            case DiagnosticSeverity::Warning:
            {
                Log << termcolor::bright_yellow;
                break;
            }

            case DiagnosticSeverity::Error:
            {
                Log << termcolor::bright_red;
                break;
            }
        }
    }

    static auto LogSnippet(const Diagnostic& diagnostic) -> void
    {
        const auto displayInfo = CalculateSnippetDisplayInfo(
            diagnostic.OptSrcLocation.value()
        );

        Log << termcolor::bright_blue << " | " << termcolor::reset;

        Log << displayInfo.String;
        Log << "\n";

        Log << termcolor::bright_blue << " | ";

        LogSeverityColor(diagnostic.Severity);
        Log << displayInfo.Highlight << termcolor::reset;
        Log << "\n";
    }

    static auto LogDiagnostic(const Diagnostic& diagnostic) -> void
    {
        LogSeverityColor(diagnostic.Severity);

        switch (diagnostic.Severity)
        {
            case DiagnosticSeverity::Info:
            {
                Log << "info";
                break;
            }

            case DiagnosticSeverity::Note:
            {
                Log << "note";
                break;
            }

            case DiagnosticSeverity::Warning:
            {
                Log << "warning";
            }

            case DiagnosticSeverity::Error:
            {
                Log << "error";
            }
        }

        Log << termcolor::reset << ": ";
        Log << diagnostic.Message << "\n";
        
        if (diagnostic.OptSrcLocation.has_value())
        {
            Log << termcolor::bright_blue << "--> ";
            Log << termcolor::reset;
            Log << diagnostic.OptSrcLocation.value().Buffer->FormatLocation(
                diagnostic.OptSrcLocation.value()
            );
            Log << "\n";

            LogSnippet(diagnostic);
        }
    }

    static auto LogDiagnosticGroup(
        const std::shared_ptr<const DiagnosticGroup>& diagnosticGroup
    ) -> void
    {
        std::for_each(
            begin(diagnosticGroup->Diagnostics),
            end  (diagnosticGroup->Diagnostics),
            [&](const Diagnostic& diagnostic)
            {
                LogDiagnostic(diagnostic);
            }
        );
    }

    auto LogGlobalDiagnostics(
        const DiagnosticBag& diagnostics,
        const size_t lastLogSize
    ) -> void
    {
        std::for_each(
            begin(diagnostics.GetDiagnosticGroups()) + lastLogSize,
            end  (diagnostics.GetDiagnosticGroups()),
            [&](const std::shared_ptr<const DiagnosticGroup>& diagnosticGroup)
            {
                const bool isFirstDiagnostic =
                    diagnosticGroup ==
                    *begin(diagnostics.GetDiagnosticGroups());

                if (!isFirstDiagnostic)
                {
                    Log << "\n";
                }

                LogDiagnosticGroup(diagnosticGroup);
            }
        );
    }
}
