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

    static auto CreateNativeTypeName(
        const SrcLocation& srcLocation,
        const NativeType& nativeType
    ) -> std::vector<std::shared_ptr<const Token>>
    {
        const auto name = nativeType.CreateFullyQualifiedName(
            srcLocation
        );

        std::vector<std::shared_ptr<const Token>> tokens{};

        ACE_ASSERT(name.IsGlobal);
        tokens.push_back(std::make_shared<const Token>(
            srcLocation,
            TokenKind::ColonColon
        ));

        for (size_t i = 0; i < name.Sections.size(); i++)
        {
            const auto& section = name.Sections.at(i);

            if (i != 0)
            {
                tokens.emplace_back(std::make_shared<const Token>(
                    section.Name.SrcLocation,
                    TokenKind::ColonColon
                ));
            }

            tokens.emplace_back(std::make_shared<const Token>(
                section.Name.SrcLocation,
                TokenKind::Ident,
                section.Name.String
            ));
        }

        return tokens;
    }

    static auto CreateKeyword(
        const Lexer& lexer,
        const std::string& string,
        const SrcLocation& srcLocation
    ) -> std::optional<std::vector<std::shared_ptr<const Token>>>
    {
        const auto& natives = lexer.GetFileBuffer()->GetCompilation()->Natives;

        Token token
        {
            srcLocation,
            TokenKind::Ident,
        };

        if (string == Keyword::If)
        {
            token.Kind = TokenKind::IfKeyword;
        }
        else if (string == Keyword::Else)
        {
            token.Kind = TokenKind::ElseKeyword;
        }
        else if (string == Keyword::Elif)
        {
            token.Kind = TokenKind::ElifKeyword;
        }
        else if (string == Keyword::While)
        {
            token.Kind = TokenKind::WhileKeyword;
        }
        else if (string == Keyword::Return)
        {
            token.Kind = TokenKind::ReturnKeyword;
        }
        else if (string == Keyword::Struct)
        {
            token.Kind = TokenKind::StructKeyword;
        }
        else if (string == Keyword::Op)
        {
            token.Kind = TokenKind::OpKeyword;
        }
        else if (string == Keyword::Public)
        {
            token.Kind = TokenKind::PublicKeyword;
        }
        else if (string == Keyword::Extern)
        {
            token.Kind = TokenKind::ExternKeyword;
        }
        else if (string == Keyword::Cast)
        {
            token.Kind = TokenKind::CastKeyword;
        }
        else if (string == Keyword::Exit)
        {
            token.Kind = TokenKind::ExitKeyword;
        }
        else if (string == Keyword::Assert)
        {
            token.Kind = TokenKind::AssertKeyword;
        }
        else if (string == Keyword::Module)
        {
            token.Kind = TokenKind::ModuleKeyword;
        }
        else if (string == Keyword::Impl)
        {
            token.Kind = TokenKind::ImplKeyword;
        }
        else if (string == Keyword::AddressOf)
        {
            token.Kind = TokenKind::AddressOfKeyword;
        }
        else if (string == Keyword::SizeOf)
        {
            token.Kind = TokenKind::SizeOfKeyword;
        }
        else if (string == Keyword::DerefAs)
        {
            token.Kind = TokenKind::DerefAsKeyword;
        }
        else if (string == Keyword::Box)
        {
            token.Kind = TokenKind::BoxKeyword;
        }
        else if (string == Keyword::Unbox)
        {
            token.Kind = TokenKind::UnboxKeyword;
        }
        else if (string == Keyword::True)
        {
            token.Kind = TokenKind::TrueKeyword;
        }
        else if (string == Keyword::False)
        {
            token.Kind = TokenKind::FalseKeyword;
        }

        else if (string == Keyword::Int8)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Int8
            );
        }
        else if (string == Keyword::Int16)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Int16
            );
        }
        else if (string == Keyword::Int32)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Int32
            );
        }
        else if (string == Keyword::Int64)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Int64
            );
        }
        else if (string == Keyword::UInt8)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->UInt8
            );
        }
        else if (string == Keyword::UInt16)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->UInt16
            );
        }
        else if (string == Keyword::UInt32)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->UInt32
            );
        }
        else if (string == Keyword::UInt64)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->UInt64
            );
        }
        else if (string == Keyword::Int)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Int
            );
        }
        else if (string == Keyword::Float32)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Float32
            );
        }
        else if (string == Keyword::Float64)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Float64
            );
        }
        else if (string == Keyword::Bool)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Bool
            );
        }
        else if (string == Keyword::Void)
        {
            return CreateNativeTypeName(
                srcLocation,
                natives->Void
            );
        }
        else
        {
            return std::nullopt;
        }

        return std::vector{ std::make_shared<const Token>(token) };
    }

    static auto CreateNumericLiteralTokenKind(
        const SrcLocation& srcLocation,
        const std::string& suffix
    ) -> Expected<TokenKind>
    {
        if (suffix == "i8")  return TokenKind::Int8;
        if (suffix == "i16") return TokenKind::Int16;
        if (suffix == "i32") return TokenKind::Int32;
        if (suffix == "i64") return TokenKind::Int64;
        
        if (suffix == "u8")  return TokenKind::UInt8;
        if (suffix == "u16") return TokenKind::UInt16;
        if (suffix == "u32") return TokenKind::UInt32;
        if (suffix == "u64") return TokenKind::UInt64;
        if (suffix == "u64") return TokenKind::UInt64;

        if (suffix == "f32") return TokenKind::Float32;
        if (suffix == "f64") return TokenKind::Float64;

        return DiagnosticBag{}.Add(CreateUnknownNumericLiteralTypeSuffixError(
            srcLocation
        ));
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
    ) -> std::vector<std::shared_ptr<const Token>>
    {
        const auto beginSrcLocation = lexer.GetSrcLocation();

        ACE_ASSERT(IsIdentBegin(lexer));
        std::string string{ lexer.Eat() };
        while (IsIdentContinue(lexer))
        {
            string += lexer.Eat();
        }

        const auto identToken = std::make_shared<const Token>(
            SrcLocation{ beginSrcLocation, lexer.GetLastSrcLocation() },
            TokenKind::Ident,
            string
        );

        const auto optKeywordTokens = CreateKeyword(
            lexer,
            string,
            identToken->SrcLocation
        );

        const auto tokens = optKeywordTokens.has_value() ?
            optKeywordTokens.value() :
            std::vector{ identToken };

        return tokens;
    }

    static auto LexNumericLiteral(
        Lexer& lexer
    ) -> Diagnosed<std::shared_ptr<const Token>>
    {
        DiagnosticBag diagnostics{};

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
            const auto expTokenKind = CreateNumericLiteralTokenKind(
                suffixSrcLocation,
                suffix
            );
            diagnostics.Add(expTokenKind);
            if (expTokenKind)
            {
                tokenKind = expTokenKind.Unwrap();
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

        return
        {
            token,
            diagnostics,
        };
    }

    static auto LexDefaultTokenKind(
        Lexer& lexer
    ) -> Expected<TokenKind>
    {
        switch (lexer.Peek())
        {
            case '=':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::EqualsEquals;
                }
                else
                {
                    return TokenKind::Equals;
                }
            }

            case '+':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::PlusEquals;
                }
                else
                {
                    return TokenKind::Plus;
                }
            }

            case '-':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::MinusEquals;
                }
                else if (lexer.Peek() == '>')
                {
                    lexer.Eat();
                    return TokenKind::MinusGreaterThan;
                }
                else
                {
                    return TokenKind::Minus;
                }
            }

            case '*':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::AsteriskEquals;
                }
                else
                {
                    return TokenKind::Asterisk;
                }
            }

            case '/':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::SlashEquals;
                }
                else
                {
                    return TokenKind::Slash;
                }
            }

            case '%':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::PercentEquals;
                }
                else
                {
                    return TokenKind::Percent;
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
                        return TokenKind::LessThanLessThanEquals;
                    }
                    else
                    {
                        return TokenKind::LessThanLessThan;
                    }
                }
                else if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::LessThanEquals;
                }
                else
                {
                    return TokenKind::LessThan;
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
                        return TokenKind::GreaterThanGreaterThanEquals;
                    }
                    else
                    {
                        return TokenKind::GreaterThanGreaterThan;
                    }
                }
                else if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::GreaterThanEquals;
                }
                else
                {
                    return TokenKind::GreaterThan;
                }
            }

            case '&':
            {
                lexer.Eat();

                if (lexer.Peek() == '&')
                {
                    lexer.Eat();
                    return TokenKind::AmpersandAmpersand;
                }
                else if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::AmpersandEquals;
                }
                else
                {
                    return TokenKind::Ampersand;
                }
            }

            case '^':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::CaretEquals;
                }
                else
                {
                    return TokenKind::Caret;
                }
            }

            case '|':
            {
                lexer.Eat();

                if (lexer.Peek() == '|')
                {
                    lexer.Eat();
                    return TokenKind::VerticalBarVerticalBar;
                }
                else if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::VerticalBarEquals;
                }
                else
                {
                    return TokenKind::VerticalBar;
                }
            }

            case ':':
            {
                lexer.Eat();

                if (lexer.Peek() == ':')
                {
                    lexer.Eat();
                    return TokenKind::ColonColon;
                }
                else
                {
                    return TokenKind::Colon;
                }
            }

            case '.':
            {
                lexer.Eat();
                return TokenKind::Dot;
            }

            case ',':
            {
                lexer.Eat();
                return TokenKind::Comma;
            }

            case ';':
            {
                lexer.Eat();
                return TokenKind::Semicolon;
            }

            case '!':
            {
                lexer.Eat();

                if (lexer.Peek() == '=')
                {
                    lexer.Eat();
                    return TokenKind::ExclamationEquals;
                }
                else
                {
                    return TokenKind::Exclamation;
                }
            }

            case '~':
            {
                lexer.Eat();
                return TokenKind::Tilde;
            }

            case '(':
            {
                lexer.Eat();
                return TokenKind::OpenParen;
            }

            case ')':
            {
                lexer.Eat();
                return TokenKind::CloseParen;
            }

            case '{':
            {
                lexer.Eat();
                return TokenKind::OpenBrace;
            }

            case '}':
            {
                lexer.Eat();
                return TokenKind::CloseBrace;
            }

            case '[':
            {
                lexer.Eat();
                return TokenKind::OpenBracket;
            }

            case ']':
            {
                lexer.Eat();
                return TokenKind::CloseBracket;
            }

            default:
            {
                return DiagnosticBag{}.Add(CreateUnexpectedCharacterError(
                    lexer.GetSrcLocation()
                ));
            }
        }
    }

    static auto LexDefault(
        Lexer& lexer
    ) -> Expected<std::shared_ptr<const Token>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = lexer.GetSrcLocation();

        const auto expTokenKind = LexDefaultTokenKind(lexer);
        diagnostics.Add(expTokenKind);
        if (!expTokenKind)
        {
            return diagnostics;
        }

        const auto token = std::make_shared<const Token>(
            SrcLocation{ beginSrcLocation, lexer.GetLastSrcLocation() },
            expTokenKind.Unwrap()
        );

        return
        {
            token,
            diagnostics,
        };
    }

    static auto LexString(
        Lexer& lexer
    ) -> Diagnosed<std::shared_ptr<const Token>>
    {
        DiagnosticBag diagnostics{};

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
            diagnostics,
        };
    }

    static auto Lex(
        Lexer& lexer
    ) -> Expected<std::vector<std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnostics{};

        if (lexer.Peek() == '"')
        {
            const auto dgnString = LexString(lexer);
            diagnostics.Add(dgnString);
            return
            {
                std::vector{ dgnString.Unwrap() },
                diagnostics,
            };
        }

        if (IsIdentBegin(lexer))
        {
            return LexIdent(lexer);
        }

        if (IsNumericLiteralBegin(lexer))
        {
            const auto dgnNumericLiteral = LexNumericLiteral(lexer);
            diagnostics.Add(dgnNumericLiteral);
            return
            {
                std::vector{ dgnNumericLiteral.Unwrap() },
                diagnostics,
            };
        }

        const auto expDefault = LexDefault(lexer);
        diagnostics.Add(expDefault);
        if (!expDefault)
        {
            return diagnostics;
        }

        return
        {
            std::vector{ expDefault.Unwrap() },
            diagnostics,
        };
    }

    static auto DiscardMultiLineComment(Lexer& lexer) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

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
            diagnostics.Add(CreateUnterminatedMultiLineCommentError(
                SrcLocation{ beginSrcLocation, lexer.GetLastSrcLocation() }
            ));
        }

        return Diagnosed<void>{ diagnostics };
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

        return { DiagnosticBag{} };
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
        DiagnosticBag diagnostics{};

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
                diagnostics.Add(DiscardComment(lexer));
                continue;
            }

            const auto expTokens = Lex(lexer);
            diagnostics.Add(expTokens);
            if (expTokens)
            {
                tokens.insert(
                    end(tokens),
                    begin(expTokens.Unwrap()),
                    end  (expTokens.Unwrap())
                );
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
            diagnostics,
        };
    }
}
