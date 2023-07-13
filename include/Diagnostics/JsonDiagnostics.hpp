#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
#include "SourceLocation.hpp"
#include "String.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    inline auto CreateJsonError(
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json::exception& t_jsonException
    ) -> std::shared_ptr<const Diagnostic>
    {
        std::string what = t_jsonException.what();
        MakeLowercase(what);

        const auto closingBracketIt = std::find(
            begin(what),
            end  (what),
            ']'
        );
        ACE_ASSERT(closingBracketIt != end(what));

        auto [message, sourceLocation] = [&]() -> std::tuple<std::string, std::optional<SourceLocation>>
        {
            const auto atLinePos = what.find("at line");
            if (atLinePos == std::string::npos)
            {
                const std::string message
                {
                    closingBracketIt + 2,
                    end(what),
                };

                return { message, t_fileBuffer->CreateFirstLocation() };
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

            const auto& line = t_fileBuffer->GetLines().at(lineIndex);
            const SourceLocation sourceLocation
            {
                t_fileBuffer,
                begin(line) + characterIndex,
                begin(line) + characterIndex + 1,
            };

            return
            {
                message,
                sourceLocation,
            };
        }();

        message.at(0) = std::toupper(message.at(0));
        message = "json: " + message;

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            message
        );
    }   
}
