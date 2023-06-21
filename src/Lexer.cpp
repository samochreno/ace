#include "Lexer.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Token.hpp"
#include "Diagnostics.hpp"
#include "Keyword.hpp"
#include "Utility.hpp"
#include "FileBuffer.hpp"
#include "Compilation.hpp"
#include "SourceLocation.hpp"
#include "Measured.hpp"

namespace Ace
{
    struct ScanContext
    {
        ScanContext(
            const FileBuffer* const t_fileBuffer,
            const std::vector<std::string_view>::const_iterator& t_lineIt,
            const std::string_view::const_iterator& t_characterIt
        ) : FileBuffer{ t_fileBuffer },
            LineIterator{ t_lineIt },
            CharacterIterator{ t_characterIt }
        {
        }

        const FileBuffer* FileBuffer{};
        std::vector<std::string_view>::const_iterator LineIterator{};
        std::string_view::const_iterator CharacterIterator{};
    };

    static auto CreateNativeTypeName(
        const ScanContext& t_context,
        const SourceLocation& t_sourceLocation,
        const NativeType& t_nativeType
    ) -> std::vector<std::shared_ptr<const Token>>
    {
        const auto name = t_nativeType.GetFullyQualifiedName();

        std::vector<std::shared_ptr<const Token>> tokens{};

        ACE_ASSERT(name.IsGlobal);
        tokens.push_back(std::make_shared<const Token>(
            t_sourceLocation,
            TokenKind::ColonColon
        ));

        for (size_t i = 0; i < name.Sections.size(); i++)
        {
            if (i != 0)
            {
                tokens.emplace_back(std::make_shared<const Token>(
                    t_sourceLocation,
                    TokenKind::ColonColon
                ));
            }

            tokens.emplace_back(std::make_shared<const Token>(
                t_sourceLocation,
                TokenKind::Identifier,
                name.Sections.at(i).Name
            ));
        }

        return tokens;
    }

    static auto CreateKeyword(
        const ScanContext& t_context,
        const std::string& t_string,
        const SourceLocation& t_sourceLocation
    ) -> std::optional<std::vector<std::shared_ptr<const Token>>>
    {
        const auto& natives = t_context.FileBuffer->GetCompilation()->Natives;

        Token token
        {
            t_sourceLocation,
            TokenKind::None,
        };

        if      (t_string == Keyword::If)        token.Kind = TokenKind::IfKeyword;
        else if (t_string == Keyword::Else)      token.Kind = TokenKind::ElseKeyword;
        else if (t_string == Keyword::Elif)      token.Kind = TokenKind::ElifKeyword;
        else if (t_string == Keyword::While)     token.Kind = TokenKind::WhileKeyword;
        else if (t_string == Keyword::Return)    token.Kind = TokenKind::ReturnKeyword;
        else if (t_string == Keyword::Struct)    token.Kind = TokenKind::StructKeyword;
        else if (t_string == Keyword::Operator)  token.Kind = TokenKind::OperatorKeyword;
        else if (t_string == Keyword::Public)    token.Kind = TokenKind::PublicKeyword;
        else if (t_string == Keyword::Extern)    token.Kind = TokenKind::ExternKeyword;
        else if (t_string == Keyword::Cast)      token.Kind = TokenKind::CastKeyword;
        else if (t_string == Keyword::Exit)      token.Kind = TokenKind::ExitKeyword;
        else if (t_string == Keyword::Assert)    token.Kind = TokenKind::AssertKeyword;
        else if (t_string == Keyword::Module)    token.Kind = TokenKind::ModuleKeyword;
        else if (t_string == Keyword::Impl)      token.Kind = TokenKind::ImplKeyword;
        else if (t_string == Keyword::Expl)      token.Kind = TokenKind::ExplKeyword;
        else if (t_string == Keyword::AddressOf) token.Kind = TokenKind::AddressOfKeyword;
        else if (t_string == Keyword::SizeOf)    token.Kind = TokenKind::SizeOfKeyword;
        else if (t_string == Keyword::DerefAs)   token.Kind = TokenKind::DerefAsKeyword;
        else if (t_string == Keyword::Box)       token.Kind = TokenKind::BoxKeyword;
        else if (t_string == Keyword::Unbox)     token.Kind = TokenKind::UnboxKeyword;
        else if (t_string == Keyword::True)      token.Kind = TokenKind::TrueKeyword;
        else if (t_string == Keyword::False)     token.Kind = TokenKind::FalseKeyword;

        else if (t_string == Keyword::Int8)    return CreateNativeTypeName(t_context, t_sourceLocation, natives->Int8);
        else if (t_string == Keyword::Int16)   return CreateNativeTypeName(t_context, t_sourceLocation, natives->Int16);
        else if (t_string == Keyword::Int32)   return CreateNativeTypeName(t_context, t_sourceLocation, natives->Int32);
        else if (t_string == Keyword::Int64)   return CreateNativeTypeName(t_context, t_sourceLocation, natives->Int64);
        else if (t_string == Keyword::UInt8)   return CreateNativeTypeName(t_context, t_sourceLocation, natives->UInt8);
        else if (t_string == Keyword::UInt16)  return CreateNativeTypeName(t_context, t_sourceLocation, natives->UInt16);
        else if (t_string == Keyword::UInt32)  return CreateNativeTypeName(t_context, t_sourceLocation, natives->UInt32);
        else if (t_string == Keyword::UInt64)  return CreateNativeTypeName(t_context, t_sourceLocation, natives->UInt64);
        else if (t_string == Keyword::Int)     return CreateNativeTypeName(t_context, t_sourceLocation, natives->Int);
        else if (t_string == Keyword::Float32) return CreateNativeTypeName(t_context, t_sourceLocation, natives->Float32);
        else if (t_string == Keyword::Float64) return CreateNativeTypeName(t_context, t_sourceLocation, natives->Float64);
        else if (t_string == Keyword::Bool)    return CreateNativeTypeName(t_context, t_sourceLocation, natives->Bool);
        else if (t_string == Keyword::Void)    return CreateNativeTypeName(t_context, t_sourceLocation, natives->Void);

        else return std::nullopt;

        return std::vector{ std::make_shared<const Token>(token) };
    }

    static auto ScanIdentifier(
        const ScanContext& t_context
    ) -> Measured<std::vector<std::shared_ptr<const Token>>>
    {
        auto it = t_context.CharacterIterator;

        ACE_ASSERT(IsInAlphabet(*it) || (*it == '_'));

        while (it != end(*t_context.LineIterator))
        {
            if (
                !IsInAlphabet(*it) &&
                !IsNumber(*it) &&
                !(*it == '_')
                )
                break;

            ++it;
        }

        const SourceLocation sourceLocation
        {
            t_context.FileBuffer,
            t_context.CharacterIterator,
            it,
        };

        const std::string string{ t_context.CharacterIterator, it };

        const auto identifierToken = std::make_shared<const Token>(
            sourceLocation,
            TokenKind::Identifier,
            string
        );

        const auto optKeywordTokens = CreateKeyword(
            t_context,
            string,
            sourceLocation
        );

        const auto tokens = optKeywordTokens.has_value() ?
            optKeywordTokens.value() :
            std::vector{ identifierToken };

        return
        {
            tokens,
            Distance(t_context.CharacterIterator, it),
        };
    }

    static auto CreateNumericLiteralTokenKind(
        const std::shared_ptr<const Token>& t_suffix
    ) -> Expected<TokenKind>
    {
        DiagnosticBag diagnosticBag{};

        if (t_suffix->String == "i8")  return TokenKind::Int8;
        if (t_suffix->String == "i16") return TokenKind::Int16;
        if (t_suffix->String == "i32") return TokenKind::Int32;
        if (t_suffix->String == "i64") return TokenKind::Int64;
        
        if (t_suffix->String == "u8")  return TokenKind::UInt8;
        if (t_suffix->String == "u16") return TokenKind::UInt16;
        if (t_suffix->String == "u32") return TokenKind::UInt32;
        if (t_suffix->String == "u64") return TokenKind::UInt64;
        if (t_suffix->String == "u64") return TokenKind::UInt64;

        if (t_suffix->String == "f32") return TokenKind::Float32;
        if (t_suffix->String == "f64") return TokenKind::Float64;

        return diagnosticBag.Add<UnknownNumericLiteralTypeSuffixError>(
            t_suffix->SourceLocation
        );
    }

    static auto ScanNumericLiteralNumber(
        const ScanContext& t_context
    ) -> Measured<Token>
    {
        auto it = t_context.CharacterIterator;

        ACE_ASSERT(IsNumber(*it));

        std::string string{};
        bool hasDecimalPoint = false;
        while (
            IsNumber(*it) ||
            (!hasDecimalPoint && (*it == '.') && IsNumber(*(it + 1)))
            )
        {
            if (*it == '.')
            {
                hasDecimalPoint = true;
            }

            string += *it;
            ++it;
        }

        const SourceLocation sourceLocation
        {
            t_context.FileBuffer,
            t_context.CharacterIterator,
            it,
        };

        return
        {
            Token
            {
                sourceLocation,
                TokenKind::None,
                string,
            },
            Distance(t_context.CharacterIterator, it),
        };
    }

    static auto ScanNumericLiteralSuffix(
        const ScanContext& t_context
    ) -> Measured<std::shared_ptr<const Token>>
    {
        auto it = t_context.CharacterIterator;

        ACE_ASSERT(IsInAlphabet(*it));
        ++it;

        while (IsNumber(*it))
        {
            ++it;
        }

        const SourceLocation sourceLocation
        {
            t_context.FileBuffer,
            t_context.CharacterIterator,
            it,
        };

        const auto token = std::make_shared<const Token>(
            sourceLocation,
            TokenKind::None,
            std::string{ t_context.CharacterIterator, it }
        );

        return
        {
            token,
            Distance(t_context.CharacterIterator, it),
        };
    }

    static auto ScanNumericLiteral(
        const ScanContext& t_context
    ) -> Diagnosed<Measured<std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};
        auto it = t_context.CharacterIterator;

        auto numberToken = ScanNumericLiteralNumber({
            t_context.FileBuffer,
            t_context.LineIterator,
            it,
        });
        it += numberToken.Length;

        const auto optTypeSuffix = [&]() -> std::optional<Measured<std::shared_ptr<const Token>>>
        {
            if (!IsInAlphabet(*it))
                return std::nullopt;

            return ScanNumericLiteralSuffix({
                t_context.FileBuffer,
                t_context.LineIterator,
                it,
            });
        }();
        if (optTypeSuffix.has_value())
        {
            it += optTypeSuffix.value().Length;
        }

        const auto tokenKind = [&]() -> TokenKind
        {
            if (!optTypeSuffix.has_value())
            {
                return TokenKind::Int;
            }

            const auto expTokenKind = CreateNumericLiteralTokenKind(
                optTypeSuffix.value().Value
            );
            if (!expTokenKind)
            {
                diagnosticBag.Add(expTokenKind);
                return TokenKind::Int;
            }

            return expTokenKind.Unwrap();
        }();

        const auto decimalPointPos = numberToken.Value.String.find_first_of('.');
        if (decimalPointPos != std::string::npos)
        {
            const bool isFloatKind =
                (tokenKind == TokenKind::Float32) ||
                (tokenKind == TokenKind::Float64);

            if (!isFloatKind)
            {
                const SourceLocation sourceLocation
                {
                    t_context.FileBuffer,
                    t_context.CharacterIterator + decimalPointPos,
                    t_context.CharacterIterator + decimalPointPos + 1,
                };

                diagnosticBag.Add<DecimalPointInNonFloatNumericLiteralError>(
                    sourceLocation
                );

                numberToken.Value.String.erase(decimalPointPos);
            } 
        }

        const SourceLocation sourceLocation
        {
            t_context.FileBuffer,
            t_context.CharacterIterator,
            it, 
        };

        const auto token = std::make_shared<const Token>(
            sourceLocation,
            tokenKind,
            numberToken.Value.String
        );

        return
        {
            Measured
            {
                token,
                Distance(t_context.CharacterIterator, it),
            },
            diagnosticBag,
        };
    }

    static auto ScanDefaultTokenKind(
        const ScanContext& t_context
    ) -> Expected<Measured<TokenKind>>
    {
        DiagnosticBag diagnosticBag{};
        
        auto it = t_context.CharacterIterator;

        switch (*it)
        {
            case '=':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::EqualsEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Equals, 1 };
                }
            }

            case '+':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::PlusEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Plus, 1 };
                }
            }

            case '-':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::MinusEquals, 2 };
                }
                else if (*it == '>')
                {
                    return Measured{ TokenKind::MinusGreaterThan, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Minus, 1 };
                }
            }

            case '*':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::AsteriskEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Asterisk, 1 };
                }
            }

            case '/':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::SlashEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Slash, 1 };
                }
            }

            case '%':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::PercentEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Percent, 1 };
                }
            }

            case '<':
            {
                ++it;

                if (*it == '<')
                {
                    ++it;

                    if (*it == '=')
                    {
                        return Measured{ TokenKind::LessThanLessThanEquals, 3 };
                    }
                    else
                    {
                        return Measured{ TokenKind::LessThanLessThan, 2 };
                    }
                }
                else if (*it == '=')
                {
                    return Measured{ TokenKind::LessThanEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::LessThan, 1 };
                }
            }

            case '>':
            {
                ++it;

                if (*it == '>')
                {
                    ++it;

                    if (*it == '=')
                    {
                        return Measured{ TokenKind::GreaterThanGreaterThanEquals, 3 };
                    }
                    else
                    {
                        return Measured{ TokenKind::GreaterThanGreaterThan, 2 };
                    }
                }
                else if (*it == '=')
                {
                    return Measured{ TokenKind::GreaterThanEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::GreaterThan, 1 };
                }
            }

            case '&':
            {
                ++it;

                if (*it == '&')
                {
                    return Measured{ TokenKind::AmpersandAmpersand, 2 };
                }
                else if (*it == '=')
                {
                    return Measured{ TokenKind::AmpersandEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Ampersand, 1 };
                }
            }

            case '^':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::CaretEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Caret, 1 };
                }
            }

            case '|':
            {
                ++it;

                if (*it == '|')
                {
                    return Measured{ TokenKind::VerticalBarVerticalBar, 2 };
                }
                else if (*it == '=')
                {
                    return Measured{ TokenKind::VerticalBarEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::VerticalBar, 1 };
                }
            }

            case ':':
            {
                ++it;

                if (*it == ':')
                {
                    return Measured{ TokenKind::ColonColon, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Colon, 1 };
                }
            }

            case '.':
            {
                return Measured{ TokenKind::Dot, 1 };
            }

            case ',':
            {
                return Measured{ TokenKind::Comma, 1 };
            }

            case ';':
            {
                return Measured{ TokenKind::Semicolon, 1 };
            }

            case '!':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::ExclamationEquals, 2 };
                }
                else
                {
                    return Measured{ TokenKind::Exclamation, 1 };
                }
            }

            case '~':
            {
                return Measured{ TokenKind::Tilde, 1 };
            }

            case '(':
            {
                return Measured{ TokenKind::OpenParen, 1 };
            }

            case ')':
            {
                return Measured{ TokenKind::CloseParen, 1 };
            }

            case '{':
            {
                return Measured{ TokenKind::OpenBrace, 1 };
            }

            case '}':
            {
                return Measured{ TokenKind::CloseBrace, 1 };
            }

            case '[':
            {
                return Measured{ TokenKind::OpenBracket, 1 };
            }

            case ']':
            {
                return Measured{ TokenKind::CloseBracket, 1 };
            }

            default:
            {
                const SourceLocation sourceLocation
                {
                    t_context.FileBuffer,
                    it,
                    it + 1,
                };

                return diagnosticBag.Add<UnexpectedCharacterError>(
                    sourceLocation
                );
            }
        }
    }

    static auto ScanDefault(
        const ScanContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};

        auto it = t_context.CharacterIterator;

        const auto expTokenKind = ScanDefaultTokenKind(t_context);
        diagnosticBag.Add(expTokenKind);
        if (!expTokenKind)
        {
            return diagnosticBag;
        }
        it += expTokenKind.Unwrap().Length;

        const SourceLocation sourceLocation
        {
            t_context.FileBuffer,
            t_context.CharacterIterator,
            it,
        };

        const auto token = std::make_shared<const Token>(
            sourceLocation,
            expTokenKind.Unwrap().Value
        );

        return
        {
            Measured
            {
                token,
                Distance(t_context.CharacterIterator, it),
            },
            diagnosticBag,
        };
    }

    static auto ScanString(
        const ScanContext& t_context
    ) -> Diagnosed<Measured<std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};

        auto it = t_context.CharacterIterator;

        ACE_ASSERT(*it == '"');
        ++it;

        while (*it != '"')
        {
            if (it == end(*t_context.LineIterator))
            {
                const SourceLocation sourceLocation
                {
                    t_context.FileBuffer,
                    t_context.CharacterIterator,
                    it,
                };

                diagnosticBag.Add<UnterminatedStringLiteralError>(
                    sourceLocation
                );

                break;
            }

            ++it;
        }

        if (*it == '"')
        {
            ++it;
        }

        const SourceLocation sourceLocation
        {
            t_context.FileBuffer,
            t_context.CharacterIterator,
            it,
        };

        const auto token = std::make_shared<const Token>(
            sourceLocation,
            TokenKind::String,
            std::string{ t_context.CharacterIterator + 1, it }
        );

        return
        {
            Measured
            {
                token,
                Distance(t_context.CharacterIterator, it),
            },
            diagnosticBag,
        };
    }

    static auto Scan(
        const ScanContext& t_context
    ) -> Expected<Measured<std::vector<std::shared_ptr<const Token>>>>
    {
        DiagnosticBag diagnosticBag{};

        const auto character = *t_context.CharacterIterator;

        if (character == '"')
        {
            const auto dgnString = ScanString(t_context);
            diagnosticBag.Add(dgnString);
            return
            {
                Measured
                {
                    std::vector{ dgnString.Unwrap().Value },
                    dgnString.Unwrap().Length,
                },
                diagnosticBag,
            };
        }

        if (IsInAlphabet(character) || (character == '_'))
        {
            return ScanIdentifier(t_context);
        }

        if (IsNumber(character))
        {
            const auto dgnNumericLiteral = ScanNumericLiteral(t_context);
            diagnosticBag.Add(dgnNumericLiteral);
            return
            {
                Measured
                {
                    std::vector{ dgnNumericLiteral.Unwrap().Value },
                    dgnNumericLiteral.Unwrap().Length
                },
                diagnosticBag,
            };
        }

        const auto expDefault = ScanDefault(t_context);
        diagnosticBag.Add(expDefault);
        if (!expDefault)
        {
            return diagnosticBag;
        }

        return
        {
            Measured
            {
                std::vector{ expDefault.Unwrap().Value },
                expDefault.Unwrap().Length,
            },
            diagnosticBag,
        };
    }

    Lexer::Lexer(const FileBuffer* const t_fileBuffer)
        : m_FileBuffer{ t_fileBuffer }
    {
        m_LineIterator = begin(m_FileBuffer->GetLines());
        ResetCharacterIterator();
    }

    auto Lexer::EatTokens() -> Diagnosed<std::vector<std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<std::shared_ptr<const Token>> tokens{};

        while (!IsEndOfFile())
        {
            if (GetLine().empty())
            {
                EatLine();
                continue;
            }

            EatWhitespace();

            if (IsEndOfLine())
            {
                EatLine();
                continue;
            }

            if (IsCommentStart())
            {
                EatComment();
                continue;
            }

            const auto expTokens = ScanTokenSequence();
            diagnosticBag.Add(expTokens);

            if (expTokens)
            {
                tokens.insert(
                    end(tokens),
                    begin(expTokens.Unwrap().Value),
                    end  (expTokens.Unwrap().Value)
                );

                EatCharactersUntil(
                    tokens.back()->SourceLocation.CharacterEndIterator
                );
            }
            else
            {
                EatCharacter();
            }
        }

        tokens.push_back(std::make_shared<const Token>(
            SourceLocation{},
            TokenKind::EndOfFile
        ));

        return Diagnosed
        {
            tokens,
            diagnosticBag,
        };
    }

    static auto IsWhitespace(const char& t_character) -> bool
    {
        return 
            (t_character == ' ') ||
            (t_character == '\f') || 
            (t_character == '\t') ||
            (t_character == '\v');
    }

    auto Lexer::EatCharacter() -> void
    {
        EatCharacters(1);
    }

    auto Lexer::EatCharacters(const size_t& t_count) -> void
    {
        m_CharacterIterator += t_count;
    }

    auto Lexer::EatCharactersUntil(
        const std::string_view::const_iterator& t_it
    ) -> void
    {
        while (m_CharacterIterator != t_it)
        {
            m_CharacterIterator++;
        }
    }

    auto Lexer::EatWhitespace() -> void
    {
        while (IsWhitespace(GetCharacter()))
        {
            EatCharacter();
        }
    }

    auto Lexer::EatComment() -> Diagnosed<void>
    {
        ACE_ASSERT(GetCharacter() == '#');

        if (GetCharacter(1) == ':')
        {
            return EatMultiLineComment();
        }
        else
        {
            return EatSingleLineComment();
        }
    }

    auto Lexer::EatSingleLineComment() -> Diagnosed<void>
    {
        ACE_ASSERT(GetCharacter() == '#');
        EatCharacter();

        ACE_ASSERT(GetCharacter() != ':');

        while (!IsEndOfLine())
        {
            EatCharacter();
        }

        return {};
    }

    auto Lexer::EatMultiLineComment() -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto itBegin = m_CharacterIterator;

        ACE_ASSERT(GetCharacter() == '#');
        EatCharacter();

        ACE_ASSERT(GetCharacter() == ':');
        EatCharacter();

        while (
            (GetCharacter(0) != ':') ||
            (GetCharacter(1) != '#')
            )
        {
            if (IsEndOfFile())
            {
                const SourceLocation sourceLocation
                {
                    m_FileBuffer,
                    itBegin,
                    m_CharacterIterator,
                };

                return diagnosticBag.Add<UnterminatedMultiLineCommentError>(
                    sourceLocation
                );
            }

            EatCharacter();
        }

        EatCharacters(2);
        return {};
    }

    auto Lexer::EatLine() -> void
    {
        m_LineIterator++;
        ResetCharacterIterator();
    }

    auto Lexer::ResetCharacterIterator() -> void
    {
        m_CharacterIterator = begin(GetLine());
    }

    auto Lexer::ScanTokenSequence() const -> Expected<Measured<std::vector<std::shared_ptr<const Token>>>>
    {
        return Ace::Scan({
            m_FileBuffer,
            m_LineIterator,
            m_CharacterIterator
        });
    }

    auto Lexer::GetCharacter() const -> char
    {
        return *m_CharacterIterator;
    }

    auto Lexer::GetCharacter(const size_t& t_offset) const -> char
    {
        const auto remainingCharactersCount = std::distance(
            m_CharacterIterator,
            end(GetLine())
        );
        if (t_offset > remainingCharactersCount)
        {
            return '\0';
        }

        return *(m_CharacterIterator + t_offset);
    }

    auto Lexer::GetLine() const -> const std::string_view&
    {
        return *m_LineIterator;
    }

    auto Lexer::IsEndOfLine() const -> bool
    {
        return m_CharacterIterator == end(GetLine());
    }

    auto Lexer::IsEndOfFile() const -> bool
    {
        const bool isLastLine =
            m_LineIterator == (end(m_FileBuffer->GetLines()) - 1);

        return isLastLine && IsEndOfLine();
    }

    auto Lexer::IsCommentStart() const -> bool
    {
        return GetCharacter() == '#';
    }
}
