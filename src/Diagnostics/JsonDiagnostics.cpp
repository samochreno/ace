#include "Diagnostics/JsonDiagnostics.hpp"

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "String.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto CreateJsonError(
        const FileBuffer* const fileBuffer,
        const nlohmann::json::exception& jsonException
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        std::string what = jsonException.what();
        MakeLowercase(what);

        const auto closingBracketIt = std::find(
            begin(what),
            end  (what),
            ']'
        );
        ACE_ASSERT(closingBracketIt != end(what));

        auto [message, srcLocation] = [&]() -> std::tuple<std::string, std::optional<SrcLocation>>
        {
            const auto atLinePos = what.find("at line");
            if (atLinePos == std::string::npos)
            {
                const std::string message
                {
                    closingBracketIt + 2,
                    end(what),
                };

                return { message, fileBuffer->CreateFirstLocation() };
            }

            const auto colonIt = std::find(
                begin(what),
                end  (what),
                ':'
            );

            const std::string message
            {
                colonIt + 2,
                end(what),
            };

            auto it = begin(what) + atLinePos + 8;

            std::string lineString{};
            while (IsNumber(*it))
            {
                lineString += *it;
                ++it;
            }

            it += 9;

            std::string columnString{};
            while (IsNumber(*it))
            {
                columnString += *it;
                ++it;
            }

            const auto lineIndex =
                static_cast<size_t>(std::stoi(lineString) - 1);

            const auto characterIndex =
                static_cast<size_t>(std::stoi(columnString));

            const auto& line = fileBuffer->GetLines().at(lineIndex);
            const SrcLocation srcLocation
            {
                fileBuffer,
                begin(line) + characterIndex,
                begin(line) + characterIndex + 1,
            };

            return
            {
                message,
                srcLocation,
            };
        }();

        message.at(0) = std::toupper(message.at(0));
        message = "json: " + message;

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }   
}
