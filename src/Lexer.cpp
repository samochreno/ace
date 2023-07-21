#include "Lexer.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Diagnostic.hpp"
#include "Diagnostics/LexDiagnostics.hpp"
#include "Token.hpp"
#include "Keyword.hpp"
#include "String.hpp"
#include "FileBuffer.hpp"
#include "Compilation.hpp"
#include "SourceLocation.hpp"
#include "Measured.hpp"

namespace Ace
{
    struct ScanContext
    {
        ScanContext(
            const FileBuffer* const fileBuffer,
            const std::vector<std::string_view>::const_iterator lineIt,
            const std::string_view::const_iterator characterIt
        ) : FileBuffer{ fileBuffer },
            LineIterator{ lineIt },
            CharacterIterator{ characterIt }
        {
        }

        const FileBuffer* FileBuffer{};
        std::vector<std::string_view>::const_iterator LineIterator{};
        std::string_view::const_iterator CharacterIterator{};
    };

    static auto CreateNativeTypeName(
        const ScanContext& context,
        const SourceLocation& sourceLocation,
        const NativeType& nativeType
    ) -> std::vector<std::shared_ptr<const Token>>
    {
        const auto name = nativeType.CreateFullyQualifiedName(
            sourceLocation
        );

        std::vector<std::shared_ptr<const Token>> tokens{};

        ACE_ASSERT(name.IsGlobal);
        tokens.push_back(std::make_shared<const Token>(
            sourceLocation,
            TokenKind::ColonColon
        ));

        for (size_t i = 0; i < name.Sections.size(); i++)
        {
            const auto& section = name.Sections.at(i);

            if (i != 0)
            {
                tokens.emplace_back(std::make_shared<const Token>(
                    section.Name.SourceLocation,
                    TokenKind::ColonColon
                ));
            }

            tokens.emplace_back(std::make_shared<const Token>(
                section.Name.SourceLocation,
                TokenKind::Identifier,
                section.Name.String
            ));
        }

        return tokens;
    }

    static auto CreateKeyword(
        const ScanContext& context,
        const std::string& string,
        const SourceLocation& sourceLocation
    ) -> std::optional<std::vector<std::shared_ptr<const Token>>>
    {
        const auto& natives = context.FileBuffer->GetCompilation()->Natives;

        Token token
        {
            sourceLocation,
            TokenKind::Identifier,
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
                context,
                sourceLocation,
                natives->Int8
            );
        }
        else if (string == Keyword::Int16)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->Int16
            );
        }
        else if (string == Keyword::Int32)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->Int32
            );
        }
        else if (string == Keyword::Int64)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->Int64
            );
        }
        else if (string == Keyword::UInt8)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->UInt8
            );
        }
        else if (string == Keyword::UInt16)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->UInt16
            );
        }
        else if (string == Keyword::UInt32)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->UInt32
            );
        }
        else if (string == Keyword::UInt64)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->UInt64
            );
        }
        else if (string == Keyword::Int)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->Int
            );
        }
        else if (string == Keyword::Float32)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->Float32
            );
        }
        else if (string == Keyword::Float64)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->Float64
            );
        }
        else if (string == Keyword::Bool)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->Bool
            );
        }
        else if (string == Keyword::Void)
        {
            return CreateNativeTypeName(
                context,
                sourceLocation,
                natives->Void
            );
        }
        else
        {
            return std::nullopt;
        }

        return std::vector{ std::make_shared<const Token>(token) };
    }

    static auto ScanIdentifier(
        const ScanContext& context
    ) -> Measured<std::vector<std::shared_ptr<const Token>>>
    {
        auto it = context.CharacterIterator;

        ACE_ASSERT(IsInAlphabet(*it) || (*it == '_'));

        while (it != end(*context.LineIterator))
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
            context.FileBuffer,
            context.CharacterIterator,
            it,
        };

        const std::string string{ context.CharacterIterator, it };

        const auto identifierToken = std::make_shared<const Token>(
            sourceLocation,
            TokenKind::Identifier,
            string
        );

        const auto optKeywordTokens = CreateKeyword(
            context,
            string,
            sourceLocation
        );

        const auto tokens = optKeywordTokens.has_value() ?
            optKeywordTokens.value() :
            std::vector{ identifierToken };

        return
        {
            tokens,
            std::distance(context.CharacterIterator, it),
        };
    }

    static auto CreateNumericLiteralTokenKind(
        const std::shared_ptr<const Token>& suffix
    ) -> Expected<TokenKind>
    {
        DiagnosticBag diagnosticBag{};

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

        return diagnosticBag.Add(CreateUnknownNumericLiteralTypeSuffixError(
            suffix->SourceLocation
        ));
    }

    static auto ScanNumericLiteralNumber(
        const ScanContext& context
    ) -> Measured<Token>
    {
        auto it = context.CharacterIterator;

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
            context.FileBuffer,
            context.CharacterIterator,
            it,
        };

        return
        {
            Token
            {
                sourceLocation,
                TokenKind::Int,
                string,
            },
            std::distance(context.CharacterIterator, it),
        };
    }

    static auto ScanNumericLiteralSuffix(
        const ScanContext& context
    ) -> Measured<std::shared_ptr<const Token>>
    {
        auto it = context.CharacterIterator;

        ACE_ASSERT(IsInAlphabet(*it));
        ++it;

        while (IsNumber(*it))
        {
            ++it;
        }

        const SourceLocation sourceLocation
        {
            context.FileBuffer,
            context.CharacterIterator,
            it,
        };

        return
        {
            std::make_shared<const Token>(
                sourceLocation,
                TokenKind::Identifier,
                std::string{ context.CharacterIterator, it }
            ),
            std::distance(context.CharacterIterator, it),
        };
    }

    static auto ScanNumericLiteral(
        const ScanContext& context
    ) -> Diagnosed<Measured<std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};
        auto it = context.CharacterIterator;

        auto numberToken = ScanNumericLiteralNumber({
            context.FileBuffer,
            context.LineIterator,
            it,
        });
        it += numberToken.Length;

        const auto optTypeSuffix = [&]() -> std::optional<Measured<std::shared_ptr<const Token>>>
        {
            if (!IsInAlphabet(*it))
            {
                return std::nullopt;
            }

            return ScanNumericLiteralSuffix({
                context.FileBuffer,
                context.LineIterator,
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

        const auto decimalPointPos =
            numberToken.Value.String.find_first_of('.');

        if (decimalPointPos != std::string::npos)
        {
            const bool isFloatKind =
                (tokenKind == TokenKind::Float32) ||
                (tokenKind == TokenKind::Float64);

            if (!isFloatKind)
            {
                const SourceLocation sourceLocation
                {
                    context.FileBuffer,
                    context.CharacterIterator + decimalPointPos,
                    context.CharacterIterator + decimalPointPos + 1,
                };

                diagnosticBag.Add(CreateDecimalPointInNonFloatNumericLiteralError(
                    sourceLocation
                ));

                numberToken.Value.String.erase(decimalPointPos);
            } 
        }

        const SourceLocation sourceLocation
        {
            context.FileBuffer,
            context.CharacterIterator,
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
                std::distance(context.CharacterIterator, it),
            },
            diagnosticBag,
        };
    }

    static auto ScanDefaultTokenKind(
        const ScanContext& context
    ) -> Expected<Measured<TokenKind>>
    {
        DiagnosticBag diagnosticBag{};
        
        auto it = context.CharacterIterator;

        switch (*it)
        {
            case '=':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::EqualsEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Equals, size_t{ 1 } };
                }
            }

            case '+':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::PlusEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Plus, size_t{ 1 } };
                }
            }

            case '-':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::MinusEquals, size_t{ 2 } };
                }
                else if (*it == '>')
                {
                    return Measured{ TokenKind::MinusGreaterThan, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Minus, size_t{ 1 } };
                }
            }

            case '*':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::AsteriskEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Asterisk, size_t{ 1 } };
                }
            }

            case '/':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::SlashEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Slash, size_t{ 1 } };
                }
            }

            case '%':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::PercentEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Percent, size_t{ 1 } };
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
                        return Measured{ TokenKind::LessThanLessThanEquals, size_t{ 3 } };
                    }
                    else
                    {
                        return Measured{ TokenKind::LessThanLessThan, size_t{ 2 } };
                    }
                }
                else if (*it == '=')
                {
                    return Measured{ TokenKind::LessThanEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::LessThan, size_t{ 1 } };
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
                        return Measured{ TokenKind::GreaterThanGreaterThanEquals, size_t{ 3 } };
                    }
                    else
                    {
                        return Measured{ TokenKind::GreaterThanGreaterThan, size_t{ 2 } };
                    }
                }
                else if (*it == '=')
                {
                    return Measured{ TokenKind::GreaterThanEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::GreaterThan, size_t{ 1 } };
                }
            }

            case '&':
            {
                ++it;

                if (*it == '&')
                {
                    return Measured{ TokenKind::AmpersandAmpersand, size_t{ 2 } };
                }
                else if (*it == '=')
                {
                    return Measured{ TokenKind::AmpersandEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Ampersand, size_t{ 1 } };
                }
            }

            case '^':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::CaretEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Caret, size_t{ 1 } };
                }
            }

            case '|':
            {
                ++it;

                if (*it == '|')
                {
                    return Measured{ TokenKind::VerticalBarVerticalBar, size_t{ 2 } };
                }
                else if (*it == '=')
                {
                    return Measured{ TokenKind::VerticalBarEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::VerticalBar, size_t{ 1 } };
                }
            }

            case ':':
            {
                ++it;

                if (*it == ':')
                {
                    return Measured{ TokenKind::ColonColon, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Colon, size_t{ 1 } };
                }
            }

            case '.':
            {
                return Measured{ TokenKind::Dot, size_t{ 1 } };
            }

            case ',':
            {
                return Measured{ TokenKind::Comma, size_t{ 1 } };
            }

            case ';':
            {
                return Measured{ TokenKind::Semicolon, size_t{ 1 } };
            }

            case '!':
            {
                ++it;

                if (*it == '=')
                {
                    return Measured{ TokenKind::ExclamationEquals, size_t{ 2 } };
                }
                else
                {
                    return Measured{ TokenKind::Exclamation, size_t{ 1 } };
                }
            }

            case '~':
            {
                return Measured{ TokenKind::Tilde, size_t{ 1 } };
            }

            case '(':
            {
                return Measured{ TokenKind::OpenParen, size_t{ 1 } };
            }

            case ')':
            {
                return Measured{ TokenKind::CloseParen, size_t{ 1 } };
            }

            case '{':
            {
                return Measured{ TokenKind::OpenBrace, size_t{ 1 } };
            }

            case '}':
            {
                return Measured{ TokenKind::CloseBrace, size_t{ 1 } };
            }

            case '[':
            {
                return Measured{ TokenKind::OpenBracket, size_t{ 1 } };
            }

            case ']':
            {
                return Measured{ TokenKind::CloseBracket, size_t{ 1 } };
            }

            default:
            {
                const SourceLocation sourceLocation
                {
                    context.FileBuffer,
                    it,
                    it + 1,
                };

                return diagnosticBag.Add(CreateUnexpectedCharacterError(
                    sourceLocation
                ));
            }
        }
    }

    static auto ScanDefault(
        const ScanContext& context
    ) -> Expected<Measured<std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};

        auto it = context.CharacterIterator;

        const auto expTokenKind = ScanDefaultTokenKind(context);
        diagnosticBag.Add(expTokenKind);
        if (!expTokenKind)
        {
            return diagnosticBag;
        }
        it += expTokenKind.Unwrap().Length;

        const SourceLocation sourceLocation
        {
            context.FileBuffer,
            context.CharacterIterator,
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
                std::distance(context.CharacterIterator, it),
            },
            diagnosticBag,
        };
    }

    static auto ScanString(
        const ScanContext& context
    ) -> Diagnosed<Measured<std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};

        auto it = context.CharacterIterator;

        ACE_ASSERT(*it == '"');
        ++it;

        while (*it != '"')
        {
            if (it == end(*context.LineIterator))
            {
                const SourceLocation sourceLocation
                {
                    context.FileBuffer,
                    context.CharacterIterator,
                    it,
                };

                diagnosticBag.Add(CreateUnterminatedStringLiteralError(
                    sourceLocation
                ));

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
            context.FileBuffer,
            context.CharacterIterator,
            it,
        };

        const auto token = std::make_shared<const Token>(
            sourceLocation,
            TokenKind::String,
            std::string{ context.CharacterIterator + 1, it }
        );

        return
        {
            Measured
            {
                token,
                std::distance(context.CharacterIterator, it),
            },
            diagnosticBag,
        };
    }

    static auto Scan(
        const ScanContext& context
    ) -> Expected<Measured<std::vector<std::shared_ptr<const Token>>>>
    {
        DiagnosticBag diagnosticBag{};

        const auto character = *context.CharacterIterator;

        if (character == '"')
        {
            const auto dgnString = ScanString(context);
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
            return ScanIdentifier(context);
        }

        if (IsNumber(character))
        {
            const auto dgnNumericLiteral = ScanNumericLiteral(context);
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

        const auto expDefault = ScanDefault(context);
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

    Lexer::Lexer(const FileBuffer* const fileBuffer)
        : m_FileBuffer{ fileBuffer }
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

    static auto IsWhitespace(const char character) -> bool
    {
        return 
            (character == ' ') ||
            (character == '\f') || 
            (character == '\t') ||
            (character == '\v');
    }

    auto Lexer::EatCharacter() -> void
    {
        EatCharacters(1);
    }

    auto Lexer::EatCharacters(const size_t count) -> void
    {
        m_CharacterIterator += count;
    }

    auto Lexer::EatCharactersUntil(
        const std::string_view::const_iterator it
    ) -> void
    {
        while (m_CharacterIterator != it)
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

                return diagnosticBag.Add(CreateUnterminatedMultiLineCommentError(
                    sourceLocation
                ));
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

    auto Lexer::GetCharacter(const size_t offset) const -> char
    {
        const auto remainingCharactersCount = std::distance(
            m_CharacterIterator,
            end(GetLine())
        );
        if (offset > remainingCharactersCount)
        {
            return '\0';
        }

        return *(m_CharacterIterator + offset);
    }

    auto Lexer::GetLine() const -> const std::string_view
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
