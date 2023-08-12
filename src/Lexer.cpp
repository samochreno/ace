#include "Lexer.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/LexingDiagnostics.hpp"
#include "Token.hpp"
#include "Keyword.hpp"
#include "String.hpp"
#include "FileBuffer.hpp"
#include "Compilation.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    class Lexer
    {
    public:
        Lexer(
            const FileBuffer* const fileBuffer
        ) : m_FileBuffer{ fileBuffer }
        {
            m_LineIterator = begin(m_FileBuffer->GetLines());

               m_CharacterIterator = begin(*m_LineIterator);
            m_EndCharacterIterator = end  (*m_LineIterator);

            m_SrcLocation = CreateSrcLocation();
        }
        ~Lexer() = default;

        auto GetFileBuffer() const -> const FileBuffer*
        {
            return m_FileBuffer;
        }
        auto IsEnd() const -> bool
        {
            return m_CharacterIterator == end(m_FileBuffer->GetLines().back());
        }
        auto IsEndOfLine() const -> bool
        {
            return m_CharacterIterator == m_EndCharacterIterator;
        }
        auto Peek(const size_t distance = 0) const -> char
        {
            return *(m_CharacterIterator + distance);
        }

        auto GetSrcLocation() const -> SrcLocation
        {
            return m_SrcLocation;
        }
        auto GetLastSrcLocation() const -> SrcLocation
        {
            return m_LastSrcLocation;
        }

        auto Eat() -> char
        {
            const auto beginSrcLocation = m_SrcLocation;

            if (m_CharacterIterator == m_EndCharacterIterator)
            {
                ACE_ASSERT(!IsEnd());

                m_LineIterator++;

                   m_CharacterIterator = begin(*m_LineIterator);
                m_EndCharacterIterator = end  (*m_LineIterator);
            }
            else
            {
                m_CharacterIterator++;
            }

            m_LastSrcLocation = beginSrcLocation;
            m_SrcLocation = CreateSrcLocation();

            return *beginSrcLocation.CharacterBeginIterator;
        }
        auto Eat(const size_t count) -> void
        {
            for (size_t i = 0; i < count; i++)
            {
                Eat();
            }
        }

    private:
        auto CreateSrcLocation() const -> SrcLocation
        {
            return
            {
                m_FileBuffer,
                m_CharacterIterator,
                m_CharacterIterator + 1,
            };
        }

        const FileBuffer* m_FileBuffer{};

        std::vector<std::string_view>::const_iterator m_LineIterator{};

        std::string_view::const_iterator m_CharacterIterator{};
        std::string_view::const_iterator m_EndCharacterIterator{};

        SrcLocation m_SrcLocation{};
        SrcLocation m_LastSrcLocation{};
    };

    static auto CreateNumericLiteralTokenKind(
        const SrcLocation& srcLocation,
        const std::string& suffix
    ) -> Expected<TokenKind>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (suffix == TokenKindToKeywordMap.at(TokenKind::Int8Keyword)) 
        {
            return Expected{ TokenKind::Int8, std::move(diagnostics) };
        }

        if (suffix == TokenKindToKeywordMap.at(TokenKind::Int16Keyword))
        {
            return Expected{ TokenKind::Int16, std::move(diagnostics) };
        }

        if (suffix == TokenKindToKeywordMap.at(TokenKind::Int32Keyword))
        {
            return Expected{ TokenKind::Int32, std::move(diagnostics) };
        }

        if (suffix == TokenKindToKeywordMap.at(TokenKind::Int64Keyword))
        {
            return Expected{ TokenKind::Int64, std::move(diagnostics) };
        }
        
        if (suffix == TokenKindToKeywordMap.at(TokenKind::UInt8Keyword))
        {
            return Expected{ TokenKind::UInt8, std::move(diagnostics) };
        }

        if (suffix == TokenKindToKeywordMap.at(TokenKind::UInt16Keyword))
        {
            return Expected{ TokenKind::UInt16, std::move(diagnostics) };
        }

        if (suffix == TokenKindToKeywordMap.at(TokenKind::UInt32Keyword))
        {
            return Expected{ TokenKind::UInt32, std::move(diagnostics) };
        }

        if (suffix == TokenKindToKeywordMap.at(TokenKind::UInt64Keyword))
        {
            return Expected{ TokenKind::UInt64, std::move(diagnostics) };
        }

        if (suffix == TokenKindToKeywordMap.at(TokenKind::Float32Keyword))
        {
            return Expected{ TokenKind::Float32, std::move(diagnostics) };
        }
    
        if (suffix == TokenKindToKeywordMap.at(TokenKind::Float64Keyword))
        {
            return Expected{ TokenKind::Float64, std::move(diagnostics) };
        }

        diagnostics.Add(CreateUnknownNumericLiteralTypeSuffixError(
            srcLocation
        ));
        return std::move(diagnostics);
    }

    static auto IsIdentBegin(const Lexer& lexer) -> bool
    {
        return
            IsInAlphabet(lexer.Peek()) ||
            (lexer.Peek() == '_');
    }

    static auto IsIdentContinue(const Lexer& lexer) -> bool
    {
        return
            IsIdentBegin(lexer) ||
            IsNumber(lexer.Peek());
    }

    static auto IsNumericLiteralNumberBegin(const Lexer& lexer) -> bool
    {
        return IsNumber(lexer.Peek());
    }

    static auto IsNumericLiteralNumberContinue(
        const Lexer& lexer,
        const bool hasDecimalPoint
    ) -> bool
    {
        if (IsNumericLiteralNumberBegin(lexer))
        {
            return true;
        }

        return
            (lexer.Peek(0) == '.') &&
            IsNumber(lexer.Peek(1)) &&
            !hasDecimalPoint;
    }

    static auto IsNumericLiteralSuffixBegin(const Lexer& lexer) -> bool
    {
        return IsInAlphabet(lexer.Peek());
    }

    static auto IsNumericLiteralSuffixContinue(const Lexer& lexer) -> bool
    {
        return IsNumber(lexer.Peek());
    }

    static auto IsNumericLiteralBegin(const Lexer& lexer) -> bool
    {
        return IsNumericLiteralNumberBegin(lexer);
    }

    static auto IsWhitespace(const Lexer& lexer) -> bool
    {
        return 
            (lexer.Peek() == ' ') ||
            (lexer.Peek() == '\f') || 
            (lexer.Peek() == '\t') ||
            (lexer.Peek() == '\v');
    }

    static auto IsCommentBegin(const Lexer& lexer) -> bool
    {
        return lexer.Peek() == '#';
    }

    static auto IsMultiLineCommentEnd(const Lexer& lexer) -> bool
    {
        return
            (lexer.Peek(0) == ':') &&
            (lexer.Peek(1) == '#');
    }

    static auto LexIdent(
        Lexer& lexer
    ) -> std::shared_ptr<const Token>
    {
        const auto beginSrcLocation = lexer.GetSrcLocation();

        ACE_ASSERT(IsIdentBegin(lexer));
        std::string string{ lexer.Eat() };
        while (IsIdentContinue(lexer))
        {
            string += lexer.Eat();
        }

        const SrcLocation srcLocation
        {
            beginSrcLocation,
            lexer.GetLastSrcLocation(),
        };

        const auto keywordTokenKindIt = KeywordToTokenKindMap.find(string);
        if (keywordTokenKindIt != end(KeywordToTokenKindMap))
        {
            return std::make_shared<const Token>(
                srcLocation,
                keywordTokenKindIt->second
            );
        }

        return std::make_shared<const Token>(
            srcLocation,
            TokenKind::Ident,
            string
        );
    }

    static auto LexNumericLiteral(
        Lexer& lexer
    ) -> Diagnosed<std::shared_ptr<const Token>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = lexer.GetSrcLocation();

        ACE_ASSERT(IsNumericLiteralNumberBegin(lexer));
        std::string numberString{ lexer.Eat() };
        bool hasDecimalPoint = false;
        while (IsNumericLiteralNumberContinue(lexer, hasDecimalPoint))
        {
            if (lexer.Peek() == '.')
            {
                hasDecimalPoint = true;
            }

            numberString += lexer.Eat();
        }

        const auto suffixBeginSrcLocation = lexer.GetSrcLocation();
        std::string suffix{};
        if (IsNumericLiteralSuffixBegin(lexer))
        {
            suffix += lexer.Eat();
            while (IsNumericLiteralSuffixContinue(lexer))
            {
                suffix += lexer.Eat();
            }
        }

        auto tokenKind = TokenKind::Int;
        if (!suffix.empty())
        {
            const SrcLocation suffixSrcLocation
            {
                suffixBeginSrcLocation,
                lexer.GetLastSrcLocation(),
            };
            const auto optTokenKind = diagnostics.Collect(CreateNumericLiteralTokenKind(
                suffixSrcLocation,
                suffix
            ));
            if (optTokenKind.has_value())
            {
                tokenKind = optTokenKind.value();
            }
        }

        const auto decimalPointPos = numberString.find_first_of('.');
        if (decimalPointPos != std::string::npos)
        {
            const bool isFloatKind =
                (tokenKind == TokenKind::Float32) ||
                (tokenKind == TokenKind::Float64);

            if (!isFloatKind)
            {
                const SrcLocation decimalPointSrcLocation
                {
                    beginSrcLocation.Buffer,
                    beginSrcLocation.CharacterBeginIterator + decimalPointPos,
                    beginSrcLocation.CharacterBeginIterator + decimalPointPos + 1,
                };

                diagnostics.Add(CreateDecimalPointInNonFloatNumericLiteralError(
                    decimalPointSrcLocation
                ));

                numberString.erase(decimalPointPos);
            } 
        }

        const auto token = std::make_shared<const Token>(
            SrcLocation{ beginSrcLocation, lexer.GetLastSrcLocation() },
            tokenKind,
            numberString
        );

        return Diagnosed
        {
            token,
            std::move(diagnostics),
        };
    }

    static auto LexDefaultTokenKind(
        Lexer& lexer
    ) -> Expected<TokenKind>
    {
        auto diagnostics = DiagnosticBag::Create();

        switch (lexer.Peek())
        {
            case '=':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::EqualsEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Equals,
                        std::move(diagnostics),
                    };
                }
            }

            case '+':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::PlusEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected {
                        TokenKind::Plus,
                        std::move(diagnostics),
                    };
                }
            }

            case '-':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::MinusEquals,
                        std::move(diagnostics),
                    };
                }
                else if (lexer.Peek() == '>')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::MinusGreaterThan,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Minus,
                        std::move(diagnostics),
                    };
                }
            }

            case '*':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::AsteriskEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Asterisk,
                        std::move(diagnostics),
                    };
                }
            }

            case '/':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::SlashEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Slash,
                        std::move(diagnostics),
                    };
                }
            }

            case '%':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::PercentEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Percent,
                        std::move(diagnostics),
                    };
                }
            }

            case '<':
            {
                lexer.Eat();

                if (lexer.Peek() == '<')
                {
                    lexer.Eat();

                    if (lexer.Peek() == '=')
                    {
                        lexer.Eat();
                        return Expected
                        {
                            TokenKind::LessThanLessThanEquals,
                            std::move(diagnostics),
                        };
                    }
                    else
                    {
                        return Expected
                        {
                            TokenKind::LessThanLessThan,
                            std::move(diagnostics),
                        };
                    }
                }
                else if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::LessThanEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::LessThan,
                        std::move(diagnostics),
                    };
                }
            }

            case '>':
            {
                lexer.Eat();

                if (lexer.Peek() == '>')
                {
                    lexer.Eat();

                    if (lexer.Peek() == '=')
                    {
                        lexer.Eat();
                        return Expected
                        {
                            TokenKind::GreaterThanGreaterThanEquals,
                            std::move(diagnostics),
                        };
                    }
                    else
                    {
                        return Expected
                        {
                            TokenKind::GreaterThanGreaterThan,
                            std::move(diagnostics),
                        };
                    }
                }
                else if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::GreaterThanEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::GreaterThan,
                        std::move(diagnostics),
                    };
                }
            }

            case '&':
            {
                lexer.Eat();

                if (lexer.Peek() == '&')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::AmpersandAmpersand,
                        std::move(diagnostics),
                    };
                }
                else if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::AmpersandEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Ampersand,
                        std::move(diagnostics),
                    };
                }
            }

            case '^':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::CaretEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Caret,
                        std::move(diagnostics),
                    };
                }
            }

            case '|':
            {
                lexer.Eat();

                if (lexer.Peek() == '|')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::VerticalBarVerticalBar,
                        std::move(diagnostics),
                    };
                }
                else if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::VerticalBarEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::VerticalBar,
                        std::move(diagnostics),
                    };
                }
            }

            case ':':
            {
                lexer.Eat();

                if (lexer.Peek() == ':')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::ColonColon,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Colon,
                        std::move(diagnostics),
                    };
                }
            }

            case '.':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::Dot,
                    std::move(diagnostics),
                };
            }

            case ',':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::Comma,
                    std::move(diagnostics),
                };
            }

            case ';':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::Semicolon,
                    std::move(diagnostics),
                };
            }

            case '!':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return Expected
                    {
                        TokenKind::ExclamationEquals,
                        std::move(diagnostics),
                    };
                }
                else
                {
                    return Expected
                    {
                        TokenKind::Exclamation,
                        std::move(diagnostics),
                    };
                }
            }

            case '~':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::Tilde,
                    std::move(diagnostics),
                };
            }

            case '(':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::OpenParen,
                    std::move(diagnostics),
                };
            }

            case ')':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::CloseParen,
                    std::move(diagnostics),
                };
            }

            case '{':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::OpenBrace,
                    std::move(diagnostics),
                };
            }

            case '}':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::CloseBrace,
                    std::move(diagnostics),
                };
            }

            case '[':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::OpenBracket,
                    std::move(diagnostics),
                };
            }

            case ']':
            {
                lexer.Eat();
                return Expected
                {
                    TokenKind::CloseBracket,
                    std::move(diagnostics),
                };
            }

            default:
            {
                diagnostics.Add(CreateUnexpectedCharacterError(
                    lexer.GetSrcLocation()
                ));
                return std::move(diagnostics);
            }
        }
    }

    static auto LexDefault(
        Lexer& lexer
    ) -> Expected<std::shared_ptr<const Token>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = lexer.GetSrcLocation();

        const auto optTokenKind =
            diagnostics.Collect(LexDefaultTokenKind(lexer));
        if (!optTokenKind.has_value())
        {
            return std::move(diagnostics);
        }

        const auto token = std::make_shared<const Token>(
            SrcLocation{ beginSrcLocation, lexer.GetLastSrcLocation() },
            optTokenKind.value()
        );

        return Expected
        {
            token,
            std::move(diagnostics),
        };
    }

    static auto LexString(
        Lexer& lexer
    ) -> Diagnosed<std::shared_ptr<const Token>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = lexer.GetSrcLocation();

        ACE_ASSERT(lexer.Peek() == '"');
        lexer.Eat();

        std::string value{};
        while (
            !lexer.IsEndOfLine() &&
            (lexer.Peek() != '"')
            )
        {
            value += lexer.Eat();
        }

        if (lexer.Peek() == '"')
        {
            lexer.Eat();
        }
        else
        {
            diagnostics.Add(CreateUnterminatedStringLiteralError(
                SrcLocation{ beginSrcLocation, lexer.GetLastSrcLocation() }
            ));
        }

        const auto token = std::make_shared<const Token>(
            SrcLocation{ beginSrcLocation, lexer.GetLastSrcLocation() },
            TokenKind::String,
            value
        );

        return
        {
            token,
            std::move(diagnostics),
        };
    }

    static auto Lex(Lexer& lexer) -> Expected<std::shared_ptr<const Token>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (lexer.Peek() == '"')
        {
            const auto string = diagnostics.Collect(LexString(lexer));
            return Expected{ string, std::move(diagnostics) };
        }

        if (IsIdentBegin(lexer))
        {
            return Expected{ LexIdent(lexer), std::move(diagnostics) };
        }

        if (IsNumericLiteralBegin(lexer))
        {
            const auto numericLiteral =
                diagnostics.Collect(LexNumericLiteral(lexer));

            return Expected
            {
                numericLiteral,
                std::move(diagnostics),
            };
        }

        const auto optDefault = diagnostics.Collect(LexDefault(lexer));
        if (!optDefault.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            optDefault.value(),
            std::move(diagnostics),
        };
    }

    static auto DiscardMultiLineComment(Lexer& lexer) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = lexer.GetSrcLocation();

        ACE_ASSERT(IsCommentBegin(lexer));
        lexer.Eat();

        ACE_ASSERT(lexer.Peek() == ':');
        lexer.Eat();

        while (
            !lexer.IsEnd() &&
            !IsMultiLineCommentEnd(lexer)
            )
        {
            lexer.Eat();
        }

        if (IsMultiLineCommentEnd(lexer))
        {
            lexer.Eat(2);
        }
        else
        {
            const SrcLocation srcLocation
            {
                beginSrcLocation,
                lexer.GetLastSrcLocation(),
            };

            diagnostics.Add(CreateUnterminatedMultiLineCommentError(
                srcLocation
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiscardSingleLineComment(Lexer& lexer) -> void
    {
        ACE_ASSERT(IsCommentBegin(lexer));
        lexer.Eat();

        ACE_ASSERT(lexer.Peek() != ':');

        while (!lexer.IsEndOfLine())
        {
            lexer.Eat();
        }
    }

    static auto DiscardComment(Lexer& lexer) -> Diagnosed<void>
    {
        ACE_ASSERT(IsCommentBegin(lexer));

        if (lexer.Peek(1) == ':')
        {
            return DiscardMultiLineComment(lexer);
        }
        else
        {
            DiscardSingleLineComment(lexer);
        }

        return { DiagnosticBag::Create() };
    }

    static auto DiscardWhitespace(
        Lexer& lexer
    ) -> void
    {
        while (IsWhitespace(lexer))
        {
            lexer.Eat();
        }
    }

    auto LexTokens(
        const FileBuffer* const fileBuffer
    ) -> Diagnosed<std::vector<std::shared_ptr<const Token>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        Lexer lexer{ fileBuffer };

        std::vector<std::shared_ptr<const Token>> tokens{};
        while (!lexer.IsEnd())
        {
            DiscardWhitespace(lexer);

            if (lexer.IsEndOfLine())
            {
                lexer.Eat();
                continue;
            }

            if (IsCommentBegin(lexer))
            {
                diagnostics.Collect(DiscardComment(lexer));
                continue;
            }

            auto optToken = diagnostics.Collect(Lex(lexer));
            if (optToken.has_value())
            {
                tokens.push_back(std::move(optToken.value()));
            }
            else
            {
                lexer.Eat();
            }
        }

        tokens.push_back(std::make_shared<const Token>(
            lexer.GetLastSrcLocation(),
            TokenKind::EndOfFile
        ));

        return Diagnosed
        {
            tokens,
            std::move(diagnostics),
        };
    }
}
