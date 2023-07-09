#pragma once

#include <memory>
#include <vector>
#include <string_view>

#include "Token.hpp"
#include "Diagnostics.hpp"
#include "Measured.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    class Lexer
    {
    public:
        Lexer(const FileBuffer* const t_fileBuffer);
        ~Lexer() = default;

        auto EatTokens() -> Diagnosed<std::vector<std::shared_ptr<const Token>>>;

    private:
        auto EatCharacter() -> void;
        auto EatCharacters(const size_t t_count) -> void;
        auto EatCharactersUntil(
            const std::string_view::const_iterator t_it
        ) -> void;
        auto EatWhitespace() -> void;
        auto EatComment() -> Diagnosed<void>;
        auto EatSingleLineComment() -> Diagnosed<void>;
        auto EatMultiLineComment() -> Diagnosed<void>;
        auto EatLine() -> void;

        auto ResetCharacterIterator() -> void;

        auto ScanTokenSequence() const -> Expected<Measured<std::vector<std::shared_ptr<const Token>>>>;

        auto GetCharacter()                       const -> char;
        auto GetCharacter(const size_t t_offset) const -> char;
        auto GetLine() const -> const std::string_view&;

        auto IsEndOfLine()    const -> bool;
        auto IsEndOfFile()    const -> bool;
        auto IsCommentStart() const -> bool;

        const FileBuffer* m_FileBuffer{};
        
        std::vector<std::string_view>::const_iterator m_LineIterator{};
        std::string_view::const_iterator m_CharacterIterator{};
    };
}
