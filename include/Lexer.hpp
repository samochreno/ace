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
        auto EatTokenSequence() -> std::optional<Diagnosed<std::vector<Token>, ILexerDiagnostic>>;
        auto EatCharacter() -> void;
        auto EatCharacters(const size_t& t_count) -> void;
        auto EatWhitespace() -> void;
        auto EatLine() -> void;

        auto ResetCharacterIterator() -> void;

        auto ScanTokenSequence() const -> Expected<Diagnosed<ScanResult, ILexerDiagnostic>, ILexerScanDiagnostic>;

        auto IsEndOfLine() const -> bool;
        auto IsEndOfFile() const -> bool;

        const Compilation* m_Compilation{};
        size_t m_FileIndex{};
        std::vector<std::string> m_Lines{};
        
        size_t m_LineIndex{};
        std::string::const_iterator m_CharacterIteratorBegin{};
        std::string::const_iterator m_CharacterIterator{};
        std::string::const_iterator m_CharacterIteratorEnd{};
    };
}
