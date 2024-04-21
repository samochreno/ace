#include "DiagnosticLog.hpp"

#include <string>
#include <string_view>

#include <termcolor/termcolor.hpp>

#include "Log.hpp"
#include "SrcBuffer.hpp"
#include "Assert.hpp"

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
            [](const char character) { return ' '; }
        );

        std::transform(
            begin(highlight),
            end  (highlight),
            back_inserter(highlightString),
            [](const char character) { return '^'; }
        );

        std::transform(
            end(highlight),
            end(string),
            back_inserter(highlightString),
            [](const char character) { return ' '; }
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

        ACE_ASSERT(line.size() > MaxSnippetWidth);
        ACE_ASSERT(highlight.size() < line.size());

        const size_t padding = MaxSnippetWidth - highlight.size();

        size_t leftPadding  = padding / 2;
        size_t rightPadding = padding - leftPadding;

        const size_t maxLeftPadding = std::distance(
            begin(line),
            begin(highlight)
        );
        const size_t maxRightPadding = std::distance(
            end(highlight),
            end(line)
        );

        if (leftPadding > maxLeftPadding)
        {
            leftPadding  = maxLeftPadding;
            rightPadding = padding - leftPadding;
        }
        else if (rightPadding > maxRightPadding)
        {
            rightPadding = maxRightPadding;
            leftPadding  = padding - rightPadding;
        }

        return std::string_view
        {
            begin(highlight) - leftPadding,
            end  (highlight) + rightPadding,
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

        const auto string = CalculateSnippetString(line, highlight);

        highlight = std::string_view
        {
            CreateCappedIterator(string, begin(highlight)),
            CreateCappedIterator(string, end  (highlight)),
        };

        const auto highlightString = CreateHighlightString(string, highlight);

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
                Out << termcolor::bright_green;
                break;
            }

            case DiagnosticSeverity::Warning:
            {
                Out << termcolor::bright_yellow;
                break;
            }

            case DiagnosticSeverity::Error:
            {
                Out << termcolor::bright_red;
                break;
            }
        }
    }

    static auto LogSnippet(const Diagnostic& diagnostic) -> void
    {
        const auto displayInfo = CalculateSnippetDisplayInfo(
            diagnostic.OptSrcLocation.value()
        );

        Out << termcolor::bright_blue << " | " << termcolor::reset;

        Out << displayInfo.String;
        Out << "\n";

        Out << termcolor::bright_blue << " | ";

        LogSeverityColor(diagnostic.Severity);
        Out << displayInfo.Highlight << termcolor::reset;
        Out << "\n";
    }

    static auto LogDiagnostic(const Diagnostic& diagnostic) -> void
    {
        LogSeverityColor(diagnostic.Severity);

        switch (diagnostic.Severity)
        {
            case DiagnosticSeverity::Info:
            {
                Out << "info";
                break;
            }

            case DiagnosticSeverity::Note:
            {
                Out << "note";
                break;
            }

            case DiagnosticSeverity::Warning:
            {
                Out << "warning";
            }

            case DiagnosticSeverity::Error:
            {
                Out << "error";
            }
        }

        Out << termcolor::reset << ": ";
        Out << diagnostic.Message << "\n";
        
        if (diagnostic.OptSrcLocation.has_value())
        {
            Out << termcolor::bright_blue << "--> ";
            Out << termcolor::reset;
            Out << diagnostic.OptSrcLocation.value().Buffer->FormatLocation(
                diagnostic.OptSrcLocation.value()
            );
            Out << "\n";

            LogSnippet(diagnostic);
        }
    }

    static bool HasLogged = false;

    auto LogDiagnosticGroup(const DiagnosticGroup& diagnosticGroup) -> void
    {
        if (!HasLogged)
        {
            HasLogged = true;
        }
        else
        {
            Out << "\n";
        }

        std::for_each(
            begin(diagnosticGroup.Diagnostics),
            end  (diagnosticGroup.Diagnostics),
            [&](const Diagnostic& diagnostic)
            {
                LogDiagnostic(diagnostic);
            }
        );
    }
}
