#include "Lexer.hpp"

#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#include "Token.hpp"
#include "Diagnostics.hpp"
#include "Keyword.hpp"
#include "Utility.hpp"
#include "Compilation.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    struct ScanContext
    {
        ScanContext(
            const Compilation* const t_compilation,
            const std::shared_ptr<const std::filesystem::path>& t_filePath,
            const size_t& t_lineIndex,
            const std::string::const_iterator& t_it,
            const std::string::const_iterator& t_itEnd
        ) : Compilation{ t_compilation },
            FilePath{ t_filePath },
            LineIndex{ t_lineIndex },
            Iterator{ t_it },
            IteratorEnd{ t_itEnd }
        {
        }

        const Compilation* Compilation{};
        std::shared_ptr<const std::filesystem::path> FilePath{};
        size_t LineIndex{};

        std::string::const_iterator Iterator{};
        std::string::const_iterator IteratorEnd{};
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

        for(size_t i = 0; i < name.Sections.size(); i++)
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
        const auto& natives = t_context.Compilation->Natives;

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
    ) -> std::vector<std::shared_ptr<const Token>>
    {
        auto it = t_context.Iterator;

        ACE_ASSERT(IsInAlphabet(*it) || (*it == '_'));

        while (it != t_context.IteratorEnd)
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
            t_context.Compilation,
            t_context.FilePath,
            t_context.LineIndex,
            t_context.Iterator,
            it,
        };

        const std::string string{ t_context.Iterator, it };

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

        return tokens;
    }

    static auto CreateNumericLiteralTokenKind(
        const std::shared_ptr<const Token>& t_suffix
    ) -> Expected<TokenKind, ILexerDiagnostic>
    {
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

        return std::make_shared<const InvalidNumericLiteralTypeSuffixError>(
            t_suffix->OptSourceLocation.value()
        );
    }

    static auto ScanNumericLiteralNumber(
        const ScanContext& t_context
    ) -> Token
    {
        auto it = t_context.Iterator;

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
            t_context.Compilation,
            t_context.FilePath,
            t_context.LineIndex,
            t_context.Iterator,
            it,
        };

        return Token
        {
            sourceLocation,
            TokenKind::None,
            string,
        };
    }

    static auto ScanNumericLiteralSuffix(
        const ScanContext& t_context
    ) -> std::shared_ptr<const Token>
    {
        auto it = t_context.Iterator;

        ACE_ASSERT(IsInAlphabet(*it));
        ++it;

        while (IsNumber(*it))
        {
            ++it;
        }

        const SourceLocation sourceLocation
        {
            t_context.Compilation,
            t_context.FilePath,
            t_context.LineIndex,
            t_context.Iterator,
            it,
        };

        return std::make_shared<const Token>(
            sourceLocation,
            TokenKind::None,
            std::string{ t_context.Iterator, it }
        );
    }

    static auto ScanNumericLiteral(
        const ScanContext& t_context
    ) -> Diagnosed<std::shared_ptr<const Token>, ILexerDiagnostic>
    {
        std::vector<std::shared_ptr<const ILexerDiagnostic>> diagnostics{};
        auto it = t_context.Iterator;

        auto numberToken = ScanNumericLiteralNumber({
            t_context.Compilation,
            t_context.FilePath,
            t_context.LineIndex,
            it,
            t_context.IteratorEnd,
        });
        it = numberToken.OptSourceLocation->IteratorEnd;

        const auto optTypeSuffix = [&]() -> std::optional<std::shared_ptr<const Token>>
        {
            if (!IsInAlphabet(*it))
                return std::nullopt;

            return ScanNumericLiteralSuffix({
                t_context.Compilation,
                t_context.FilePath,
                t_context.LineIndex,
                it,
                t_context.IteratorEnd
            });
        }();
        if (optTypeSuffix.has_value())
        {
            it = optTypeSuffix.value()->OptSourceLocation.value().IteratorEnd;
        }

        const auto tokenKind = [&]() -> TokenKind
        {
            if (!optTypeSuffix.has_value())
            {
                return TokenKind::Int;
            }

            const auto expTokenKind = CreateNumericLiteralTokenKind(
                optTypeSuffix.value()
            );
            if (!expTokenKind)
            {
                diagnostics.push_back(expTokenKind.GetError());
                return TokenKind::Int;
            }

            return expTokenKind.Unwrap();
        }();

        const auto decimalPointPos = numberToken.String.find_first_of('.');
        if (decimalPointPos != std::string::npos)
        {
            const bool isFloatKind =
                (tokenKind == TokenKind::Float32) ||
                (tokenKind == TokenKind::Float64);

            if (!isFloatKind)
            {
                const SourceLocation sourceLocation
                {
                    t_context.Compilation,
                    t_context.FilePath,
                    t_context.LineIndex,
                    t_context.Iterator + decimalPointPos,
                    t_context.Iterator + decimalPointPos + 1,
                };

                diagnostics.push_back(std::make_shared<const DecimalPointInNonFloatNumericLiteralError>(
                    sourceLocation
                ));

                numberToken.String.erase(decimalPointPos);
            } 
        }

        const SourceLocation sourceLocation
        {
            t_context.Compilation,
            t_context.FilePath,
            t_context.LineIndex,
            t_context.Iterator,
            it, 
        };

        const auto token = std::make_shared<const Token>(
            sourceLocation,
            tokenKind,
            numberToken.String
        );

        return Diagnosed
        {
            token,
            diagnostics,
        };
    }

    static auto ScanDefault(
        const ScanContext& t_context
    ) -> Expected<std::shared_ptr<const Token>, ILexerDiagnostic>
    {
        auto it = t_context.Iterator;

        ACE_TRY(tokenKind, ([&]() -> Expected<TokenKind, ILexerDiagnostic>
        {
            switch (*it)
            {
                case '=':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::EqualsEquals;
                    }
                    else
                    {
                        return TokenKind::Equals;
                    }
                }

                case '+':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::PlusEquals;
                    }
                    else
                    {
                        return TokenKind::Plus;
                    }
                }

                case '-':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::MinusEquals;
                    }
                    else if (*it == '>')
                    {
                        ++it;
                        return TokenKind::MinusGreaterThan;
                    }
                    else
                    {
                        return TokenKind::Minus;
                    }
                }

                case '*':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::AsteriskEquals;
                    }
                    else
                    {
                        return TokenKind::Asterisk;
                    }
                }

                case '/':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::SlashEquals;
                    }
                    else
                    {
                        return TokenKind::Slash;
                    }
                }

                case '%':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::PercentEquals;
                    }
                    else
                    {
                        return TokenKind::Percent;
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
                            ++it;
                            return TokenKind::LessThanLessThanEquals;
                        }
                        else
                        {
                            return TokenKind::LessThanLessThan;
                        }
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return TokenKind::LessThanEquals;
                    }
                    else
                    {
                        return TokenKind::LessThan;
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
                            ++it;
                            return TokenKind::GreaterThanGreaterThanEquals;
                        }
                        else
                        {
                            return TokenKind::GreaterThanGreaterThan;
                        }
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return TokenKind::GreaterThanEquals;
                    }
                    else
                    {
                        return TokenKind::GreaterThan;
                    }
                }

                case '&':
                {
                    ++it;

                    if (*it == '&')
                    {
                        ++it;
                        return TokenKind::AmpersandAmpersand;
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return TokenKind::AmpersandEquals;
                    }
                    else
                    {
                        return TokenKind::Ampersand;
                    }
                }

                case '^':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::CaretEquals;
                    }
                    else
                    {
                        return TokenKind::Caret;
                    }
                }

                case '|':
                {
                    ++it;

                    if (*it == '|')
                    {
                        ++it;
                        return TokenKind::VerticalBarVerticalBar;
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return TokenKind::VerticalBarEquals;
                    }
                    else
                    {
                        return TokenKind::VerticalBar;
                    }
                }

                case ':':
                {
                    ++it;

                    if (*it == ':')
                    {
                        ++it;
                        return TokenKind::ColonColon;
                    }
                    else
                    {
                        return TokenKind::Colon;
                    }
                }

                case '.':
                {
                    ++it;
                    return TokenKind::Dot;
                }

                case ',':
                {
                    ++it;
                    return TokenKind::Comma;
                }

                case ';':
                {
                    ++it;
                    return TokenKind::Semicolon;
                }

                case '!':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::ExclamationEquals;
                    }
                    else
                    {
                        return TokenKind::Exclamation;
                    }
                }

                case '~':
                {
                    ++it;
                    return TokenKind::Tilde;
                }


                case '(':
                {
                    ++it;
                    return TokenKind::OpenParen;
                }

                case ')':
                {
                    ++it;
                    return TokenKind::CloseParen;
                }

                case '{':
                {
                    ++it;
                    return TokenKind::OpenBrace;
                }

                case '}':
                {
                    ++it;
                    return TokenKind::CloseBrace;
                }

                case '[':
                {
                    ++it;
                    return TokenKind::OpenBracket;
                }

                case ']':
                {
                    ++it;
                    return TokenKind::CloseBracket;
                }

                default:
                {
                    const SourceLocation sourceLocation
                    {
                        t_context.Compilation,
                        t_context.FilePath,
                        t_context.LineIndex,
                        it,
                        it + 1,
                    };

                    return std::make_shared<const UnexpectedCharacterError>(
                        sourceLocation
                    );
                }
            }
        }()));

        const SourceLocation sourceLocation
        {
            t_context.Compilation,
            t_context.FilePath,
            t_context.LineIndex,
            t_context.Iterator,
            it,
        };

        const auto token = std::make_shared<const Token>(
            sourceLocation,
            tokenKind
        );

        return Expected<std::shared_ptr<const Token>, ILexerDiagnostic>
        {
            token
        };
    }

    static auto ScanString(
        const ScanContext& t_context
    ) -> Diagnosed<std::shared_ptr<const Token>, ILexerDiagnostic>
    {
        std::vector<std::shared_ptr<const ILexerDiagnostic>> diagnostics{};
        auto it = t_context.Iterator;

        ACE_ASSERT(*it == '"');
        ++it;

        while (*it != '"')
        {
            if (it == t_context.IteratorEnd)
            {
                const SourceLocation sourceLocation
                {
                    t_context.Compilation,
                    t_context.FilePath,
                    t_context.LineIndex,
                    t_context.Iterator,
                    it,
                };

                diagnostics.push_back(std::make_shared<const UnclosedStringLiteralError>(
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
            t_context.Compilation,
            t_context.FilePath,
            t_context.LineIndex,
            t_context.Iterator,
            it,
        };

        const auto token = std::make_shared<const Token>(
            sourceLocation,
            TokenKind::String,
            std::string{ t_context.Iterator + 1, it }
        );

        return Diagnosed
        {
            token,
            diagnostics,
        };
    }

    static auto Scan(
        const ScanContext& t_context
    ) -> Expected<Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>, ILexerDiagnostic>
    {
        const auto character = *t_context.Iterator;

        if (character == '"')
        {
            const auto dgnString = ScanString(t_context);
            return Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>
            {
                std::vector{ dgnString.Unwrap() },
                dgnString.GetDiagnostics(),
            };
        }

        if (IsInAlphabet(character) || (character == '_'))
        {
            return Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>
            {
                ScanIdentifier(t_context)
            };
        }

        if (IsNumber(character))
        {
            const auto dgnNumericLiteral = ScanNumericLiteral(t_context);
            return Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>
            {
                std::vector{ dgnNumericLiteral.Unwrap() },
                dgnNumericLiteral.GetDiagnostics(),
            };
        }

        ACE_TRY(dfault, ScanDefault(t_context));
        return Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>
        {
            std::vector{ dfault }
        };
    }

    Lexer::Lexer(
        const Compilation* const t_compilation,
        const std::shared_ptr<const std::filesystem::path>& t_filePath,
        const std::vector<std::string>& t_lines
    ) : m_Compilation{ t_compilation },
        m_FilePath{ t_filePath },
        m_Lines{ t_lines }
    {
        ResetCharacterIterator();
    }

    auto Lexer::EatTokens() -> Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>
    {
        std::vector<std::shared_ptr<const ILexerDiagnostic>> diagnostics{};
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

            if (const auto expDgnTokens = ScanTokenSequence())
            {
                diagnostics.insert(
                    end(diagnostics),
                    begin(expDgnTokens.Unwrap().GetDiagnostics()),
                    end  (expDgnTokens.Unwrap().GetDiagnostics())
                );

                const auto& scannedTokens = expDgnTokens.Unwrap().Unwrap();
                tokens.insert(
                    end(tokens),
                    begin(scannedTokens),
                    end  (scannedTokens)
                );
                EatCharactersUntil(
                    tokens.back()->OptSourceLocation.value().IteratorEnd
                );
            }
            else
            {
                diagnostics.push_back(expDgnTokens.GetError());
                EatCharacter();
            }
        }

        tokens.push_back(std::make_shared<const Token>(
            std::nullopt,
            TokenKind::EndOfFile
        ));

        return Diagnosed
        {
            tokens,
            diagnostics,
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

    auto Lexer::EatCharactersUntil(const std::string::const_iterator& t_it) -> void
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

    auto Lexer::EatComment() -> Diagnosed<void, ILexerDiagnostic>
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

    auto Lexer::EatSingleLineComment() -> Diagnosed<void, ILexerDiagnostic>
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

    auto Lexer::EatMultiLineComment() -> Diagnosed<void, ILexerDiagnostic>
    {
        const auto itBegin = m_CharacterIteratorBegin;

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
                    m_Compilation,
                    m_FilePath,
                    m_LineIndex,
                    itBegin,
                    m_CharacterIterator,
                };

                return Diagnosed<void, ILexerDiagnostic>
                {
                    std::vector<std::shared_ptr<const ILexerDiagnostic>>
                    {
                        std::make_shared<const UnclosedMultiLineCommentError>(
                            sourceLocation
                        )
                    }
                };
            }

            EatCharacter();
        }

        EatCharacters(2);
        return {};
    }

    auto Lexer::EatLine() -> void
    {
        m_LineIndex++;
        ResetCharacterIterator();
    }

    auto Lexer::ResetCharacterIterator() -> void
    {
        m_CharacterIteratorBegin = begin(GetLine());
        m_CharacterIterator      = begin(GetLine());
        m_CharacterIteratorEnd   = end  (GetLine());
    }

    auto Lexer::ScanTokenSequence() const -> Expected<Diagnosed<std::vector<std::shared_ptr<const Token>>, ILexerDiagnostic>, ILexerDiagnostic>
    {
        return Ace::Scan({
            m_Compilation,
            m_FilePath,
            m_LineIndex,
            m_CharacterIterator,
            m_CharacterIteratorEnd,
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
            m_CharacterIteratorEnd
        );
        if (t_offset > remainingCharactersCount)
        {
            return '\0';
        }

        return *(m_CharacterIterator + t_offset);
    }

    auto Lexer::GetLine() const -> const std::string&
    {
        return m_Lines.at(m_LineIndex);
    }

    auto Lexer::IsEndOfLine() const -> bool
    {
        return m_CharacterIterator == m_CharacterIteratorEnd;
    }

    auto Lexer::IsEndOfFile() const -> bool
    {
        const auto lastLineLindex = m_Lines.size() - 1;

        return 
            (m_LineIndex == lastLineLindex) &&
            IsEndOfLine();
    }

    auto Lexer::IsCommentStart() const -> bool
    {
        return GetCharacter() == '#';
    }
}
