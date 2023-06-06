#pragma once

#include <vector>
#include <string>

#include "Token.hpp"
#include "Diagnostics.hpp"
#include "ParseData.hpp"
#include "Compilation.hpp"

namespace Ace
{
    struct TokenKindStringPair
    {
        TokenKindStringPair(
            const TokenKind::Set& t_kind,
            const std::string& t_string
        ) : Kind{ t_kind },
            String{ t_string }
        {
        }

        TokenKind::Set Kind{};
        std::string String{};
    };

    struct ScanResult
    {
        ScanResult() = default;
        ScanResult(
            const size_t& t_length,
            const TokenKindStringPair& t_kindStringPair
        ) : Length{ t_length },
            KindStringPairs{ t_kindStringPair }
        {
        }
        ScanResult(
            const size_t& t_length,
            const std::vector<TokenKindStringPair>& t_kindStringPairs
        ) : Length{ t_length },
            KindStringPairs{ t_kindStringPairs }
        {
        }

        size_t Length{};
        std::vector<TokenKindStringPair> KindStringPairs{};
    };

    class Lexer
    {
    public:
        Lexer(
            const Compilation* const t_compilation,
            const size_t& t_fileIndex,
            const std::vector<std::string>& t_lines
        );
        ~Lexer() = default;

        auto EatTokens() -> Diagnosed<std::vector<Token>, ILexerDiagnostic>;

    private:
        auto EatCharacter() -> void;
        auto EatCharacters(const size_t& t_count) -> void;
        auto EatWhitespace() -> void;
        auto EatComment() -> Diagnosed<void, ILexerDiagnostic>;
        auto EatSingleLineComment() -> Diagnosed<void, ILexerDiagnostic>;
        auto EatMultiLineComment() -> Diagnosed<void, ILexerDiagnostic>;
        auto EatLine() -> void;

        auto ResetCharacterIterator() -> void;

        auto ScanTokenSequence() const -> Expected<Diagnosed<ScanResult, ILexerDiagnostic>, ILexerScanDiagnostic>;

        auto GetCharacter()                       const -> char;
        auto GetCharacter(const size_t& t_offset) const -> char;
        auto GetLine() const -> const std::string&;

        auto IsEndOfLine()    const -> bool;
        auto IsEndOfFile()    const -> bool;
        auto IsCommentStart() const -> bool;

        auto CreateTokens(
            const ScanResult& t_scanResult
        ) -> std::vector<Token>;

        const Compilation* m_Compilation{};
        size_t m_FileIndex{};
        std::vector<std::string> m_Lines{};
        
        size_t m_LineIndex{};
        std::string::const_iterator m_CharacterIteratorBegin{};
        std::string::const_iterator m_CharacterIterator{};
        std::string::const_iterator m_CharacterIteratorEnd{};
    };
}
