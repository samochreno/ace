#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Token.hpp"
#include "Diagnostics.hpp"
#include "File.hpp"

namespace Ace
{
    class Lexer
    {
    public:
        Lexer(
            const File* const t_file
        );
        ~Lexer() = default;

        auto EatTokens() -> Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>;

    private:
        auto EatCharacter() -> void;
        auto EatCharacters(const size_t& t_count) -> void;
        auto EatCharactersUntil(const std::string::const_iterator& t_it) -> void;
        auto EatWhitespace() -> void;
        auto EatComment() -> Diagnosed<void, ILexerDiagnostic>;
        auto EatSingleLineComment() -> Diagnosed<void, ILexerDiagnostic>;
        auto EatMultiLineComment() -> Diagnosed<void, ILexerDiagnostic>;
        auto EatLine() -> void;

        auto ResetCharacterIterator() -> void;

        auto ScanTokenSequence() const -> Expected<Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>, ILexerDiagnostic>;

        auto GetCharacter()                       const -> char;
        auto GetCharacter(const size_t& t_offset) const -> char;
        auto GetLine() const -> const std::string&;

        auto IsEndOfLine()    const -> bool;
        auto IsEndOfFile()    const -> bool;
        auto IsCommentStart() const -> bool;

        const File* m_File{};
        
        std::vector<std::string>::const_iterator m_LineIterator{};
        std::string::const_iterator m_CharacterIterator{};
    };
}
