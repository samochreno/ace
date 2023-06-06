#include "Lexer.hpp"

#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

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
            const std::string::const_iterator& t_it,
            const std::string::const_iterator& t_itEnd
        ) : Compilation{ t_compilation },
            Iterator{ t_it },
            IteratorEnd{ t_itEnd }
        {
        }

        const Compilation* const Compilation;
        std::string::const_iterator Iterator{};
        std::string::const_iterator IteratorEnd{};
    };

    static auto CreateNativeTypeName(
        const NativeType& t_nativeType
    ) -> std::vector<TokenKindStringPair>
    {
        const auto name = t_nativeType.GetFullyQualifiedName();

        std::vector<TokenKindStringPair> kindStringPairs{};

        ACE_ASSERT(name.IsGlobal);
        kindStringPairs.emplace_back(
            TokenKind::New(TokenKind::ColonColon),
            std::string{}
        );

        for (size_t i = 0; i < name.Sections.size(); i++)
        {
            if (i != 0)
            {
                kindStringPairs.emplace_back(
                    TokenKind::New(TokenKind::ColonColon),
                    std::string{}
                );
            }

            kindStringPairs.emplace_back(
                TokenKind::New(TokenKind::Identifier),
                name.Sections.at(i).Name
            );
        }

        return kindStringPairs;
    }

    static auto CreateKeyword(
        const ScanContext& t_context,
        const std::string& t_string
    ) -> std::optional<ScanResult>
    {
        const auto& natives = t_context.Compilation->Natives;

        std::vector<TokenKindStringPair> kindStringPairs{};
        kindStringPairs.emplace_back(
            TokenKind::New(TokenKind::None),
            std::string{}
        );

        if      (t_string == Keyword::If)          kindStringPairs.front().Kind = TokenKind::New(TokenKind::IfKeyword);
        else if (t_string == Keyword::Else)        kindStringPairs.front().Kind = TokenKind::New(TokenKind::ElseKeyword);
        else if (t_string == Keyword::Elif)        kindStringPairs.front().Kind = TokenKind::New(TokenKind::ElifKeyword);
        else if (t_string == Keyword::While)       kindStringPairs.front().Kind = TokenKind::New(TokenKind::WhileKeyword);
        else if (t_string == Keyword::Return)      kindStringPairs.front().Kind = TokenKind::New(TokenKind::ReturnKeyword);
        else if (t_string == Keyword::Struct)      kindStringPairs.front().Kind = TokenKind::New(TokenKind::StructKeyword);
        else if (t_string == Keyword::Operator)    kindStringPairs.front().Kind = TokenKind::New(TokenKind::OperatorKeyword);
        else if (t_string == Keyword::Public)      kindStringPairs.front().Kind = TokenKind::New(TokenKind::PublicKeyword);
        else if (t_string == Keyword::Extern)      kindStringPairs.front().Kind = TokenKind::New(TokenKind::ExternKeyword);
        else if (t_string == Keyword::Cast)        kindStringPairs.front().Kind = TokenKind::New(TokenKind::CastKeyword);
        else if (t_string == Keyword::Exit)        kindStringPairs.front().Kind = TokenKind::New(TokenKind::ExitKeyword);
        else if (t_string == Keyword::Assert)      kindStringPairs.front().Kind = TokenKind::New(TokenKind::AssertKeyword);
        else if (t_string == Keyword::Module)      kindStringPairs.front().Kind = TokenKind::New(TokenKind::ModuleKeyword);
        else if (t_string == Keyword::Impl)        kindStringPairs.front().Kind = TokenKind::New(TokenKind::ImplKeyword);
        else if (t_string == Keyword::Expl)        kindStringPairs.front().Kind = TokenKind::New(TokenKind::ExplKeyword);
        else if (t_string == Keyword::AddressOf)   kindStringPairs.front().Kind = TokenKind::New(TokenKind::AddressOfKeyword);
        else if (t_string == Keyword::SizeOf)      kindStringPairs.front().Kind = TokenKind::New(TokenKind::SizeOfKeyword);
        else if (t_string == Keyword::DerefAs)     kindStringPairs.front().Kind = TokenKind::New(TokenKind::DerefAsKeyword);
        else if (t_string == Keyword::Box)         kindStringPairs.front().Kind = TokenKind::New(TokenKind::BoxKeyword);
        else if (t_string == Keyword::Unbox)       kindStringPairs.front().Kind = TokenKind::New(TokenKind::UnboxKeyword);
        else if (t_string == Keyword::True)        kindStringPairs.front().Kind = TokenKind::New(TokenKind::TrueKeyword);
        else if (t_string == Keyword::False)       kindStringPairs.front().Kind = TokenKind::New(TokenKind::FalseKeyword);

        else if (t_string == Keyword::Int8)        kindStringPairs = CreateNativeTypeName(natives->Int8);
        else if (t_string == Keyword::Int16)       kindStringPairs = CreateNativeTypeName(natives->Int16);
        else if (t_string == Keyword::Int32)       kindStringPairs = CreateNativeTypeName(natives->Int32);
        else if (t_string == Keyword::Int64)       kindStringPairs = CreateNativeTypeName(natives->Int64);
        else if (t_string == Keyword::UInt8)       kindStringPairs = CreateNativeTypeName(natives->UInt8);
        else if (t_string == Keyword::UInt16)      kindStringPairs = CreateNativeTypeName(natives->UInt16);
        else if (t_string == Keyword::UInt32)      kindStringPairs = CreateNativeTypeName(natives->UInt32);
        else if (t_string == Keyword::UInt64)      kindStringPairs = CreateNativeTypeName(natives->UInt64);
        else if (t_string == Keyword::Int)         kindStringPairs = CreateNativeTypeName(natives->Int);
        else if (t_string == Keyword::Float32)     kindStringPairs = CreateNativeTypeName(natives->Float32);
        else if (t_string == Keyword::Float64)     kindStringPairs = CreateNativeTypeName(natives->Float64);
        else if (t_string == Keyword::Bool)        kindStringPairs = CreateNativeTypeName(natives->Bool);
        else if (t_string == Keyword::Void)        kindStringPairs = CreateNativeTypeName(natives->Void);

        else return std::nullopt;

        return ScanResult
        {
            t_string.size(),
            kindStringPairs,
        };
    }

    static auto ScanIdentifier(
        const ScanContext& t_context
    ) -> Diagnosed<ScanResult, ILexerDiagnostic>
    {
        auto it = t_context.Iterator;

        std::string string{};
        while (it != t_context.IteratorEnd)
        {
            const auto& character = *it;

            if (
                !IsInAlphabet(character) &&
                !IsNumber(character) &&
                !(character == '_')
                )
                break;

            string += character;
            ++it;
        }

        const auto optKeywordScanResult = CreateKeyword(t_context, string);

        const auto scanResult = optKeywordScanResult.has_value() ?
            optKeywordScanResult.value() :
            ScanResult
            {
                string.size(),
                TokenKindStringPair
                {
                    TokenKind::New(TokenKind::Identifier),
                    string,
                },
            };

        return Diagnosed
        {
            scanResult,
            std::vector<std::shared_ptr<const ILexerDiagnostic>>{},
        };
    }

    static auto CreateNumericLiteralTokenKind(
        const std::string& t_suffix
    ) -> std::optional<TokenKind::Set>
    {
        if (t_suffix.empty())  return TokenKind::New(TokenKind::Int);

        if (t_suffix == "i8")  return TokenKind::New(TokenKind::Int8);
        if (t_suffix == "i16") return TokenKind::New(TokenKind::Int16);
        if (t_suffix == "i32") return TokenKind::New(TokenKind::Int32);
        if (t_suffix == "i64") return TokenKind::New(TokenKind::Int64);
        
        if (t_suffix == "u8")  return TokenKind::New(TokenKind::UInt8);
        if (t_suffix == "u16") return TokenKind::New(TokenKind::UInt16);
        if (t_suffix == "u32") return TokenKind::New(TokenKind::UInt32);
        if (t_suffix == "u64") return TokenKind::New(TokenKind::UInt64);
        if (t_suffix == "u64") return TokenKind::New(TokenKind::UInt64);

        if (t_suffix == "f32") return TokenKind::New(TokenKind::Float32);
        if (t_suffix == "f64") return TokenKind::New(TokenKind::Float64);

        return std::nullopt;
    }

    static auto ScanNumericLiteralNumber(
        const ScanContext& t_context
    ) -> Diagnosed<ScanResult, ILexerDiagnostic>
    {
        std::vector<std::shared_ptr<const ILexerDiagnostic>> diagnostics{};
        auto it = t_context.Iterator;

        std::string string{};
        bool hasDecimalPoint = false;
        while (IsNumber(*it) || (*it == '.'))
        {
            const auto& character = *it;

            const bool isCharacterDecimalPoint = character == '.';
            const bool hasMultipleDecimalPoints = isCharacterDecimalPoint && hasDecimalPoint;

            if (isCharacterDecimalPoint)
            {
                hasDecimalPoint = true;
            }

            if (hasMultipleDecimalPoints)
            {
                diagnostics.push_back(std::make_shared<const MultipleDecimalPointsInNumericLiteralError>(
                    Distance(t_context.Iterator, it)
                ));
            }
            else
            {
                string += *it;
            }

            ++it;
        }

        return Diagnosed
        {
            ScanResult
            {
                Distance(t_context.Iterator, it),
                TokenKindStringPair
                {
                    TokenKind::None,
                    string,
                },
            },
            diagnostics,
        };
    }

    static auto ScanTypeSuffix(
        const ScanContext& t_context
    ) -> std::string
    {
        auto it = t_context.Iterator;

        std::string string{};
        if (IsInAlphabet(*it))
        {
            string += *it;
            ++it;

            while (IsNumber(*it))
            {
                string += *it;
                ++it;
            }
        }

        return string;
    }

    static auto ScanNumericLiteral(
        const ScanContext& t_context
    ) -> Diagnosed<ScanResult, ILexerDiagnostic>
    {
        std::vector<std::shared_ptr<const ILexerDiagnostic>> diagnostics{};
        auto it = t_context.Iterator;

        const auto dgnNumberScanResult = ScanNumericLiteralNumber({
            t_context.Compilation,
            it,
            t_context.IteratorEnd
        });
        diagnostics.insert(
            end(diagnostics),
            begin(dgnNumberScanResult.GetDiagnostics()),
            end  (dgnNumberScanResult.GetDiagnostics())
        );
        it += dgnNumberScanResult.Unwrap().Length;

        const auto typeSuffix = ScanTypeSuffix({
            t_context.Compilation,
            it,
            t_context.IteratorEnd
        });
        it += typeSuffix.size();

        const auto optTokenKind = CreateNumericLiteralTokenKind(
            typeSuffix
        );
        if (!optTokenKind.has_value())
        {
            diagnostics.push_back(std::make_shared<const InvalidNumericLiteralTypeSuffix>(
                dgnNumberScanResult.Unwrap().Length,
                typeSuffix.size()
            ));
        }

        // TODO: assert is float if has decimal point

        const auto tokenKind = optTokenKind.has_value() ?
            optTokenKind.value() : 
            TokenKind::New(TokenKind::ErrorNumber);

        const ScanResult scanResult
        {
            dgnNumberScanResult.Unwrap().Length + typeSuffix.size(),
            TokenKindStringPair
            {
                tokenKind,
                dgnNumberScanResult.Unwrap().KindStringPairs.front().String,
            },
        };

        return Diagnosed
        {
            scanResult,
            diagnostics,
        };
    }

    static auto ScanDefault(
        const ScanContext& t_context
    ) -> Expected<Diagnosed<ScanResult, ILexerDiagnostic>, ILexerScanDiagnostic>
    {
        auto it = t_context.Iterator;

        ACE_TRY(kind, ([&]() -> Expected<TokenKind::Set, ILexerScanDiagnostic>
        {
            switch (*it)
            {
                case '=':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::EqualsEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Equals);
                    }
                }

                case '+':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::PlusEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Plus);
                    }
                }

                case '-':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::MinusEquals);
                    }
                    else if (*it == '>')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::MinusGreaterThan);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Minus);
                    }
                }

                case '*':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::AsteriskEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Asterisk);
                    }
                }

                case '/':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::SlashEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Slash);
                    }
                }

                case '%':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::PercentEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Percent);
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
                            return TokenKind::New(TokenKind::LessThanLessThanEquals);
                        }
                        else
                        {
                            return TokenKind::New(TokenKind::LessThanLessThan);
                        }
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::LessThanEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::LessThan);
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
                            return TokenKind::New(TokenKind::GreaterThanGreaterThanEquals);
                        }
                        else
                        {
                            return TokenKind::New(TokenKind::GreaterThanGreaterThan);
                        }
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::GreaterThanEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::GreaterThan);
                    }
                }

                case '&':
                {
                    ++it;

                    if (*it == '&')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::AmpersandAmpersand);
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::AmpersandEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Ampersand);
                    }
                }

                case '^':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::CaretEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Caret);
                    }
                }

                case '|':
                {
                    ++it;

                    if (*it == '|')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::VerticalBarVerticalBar);
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::VerticalBarEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::VerticalBar);
                    }
                }

                case ':':
                {
                    ++it;

                    if (*it == ':')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::ColonColon);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Colon);
                    }
                }

                case '.':
                {
                    ++it;
                    return TokenKind::New(TokenKind::Dot);
                }

                case ',':
                {
                    ++it;
                    return TokenKind::New(TokenKind::Comma);
                }

                case ';':
                {
                    ++it;
                    return TokenKind::New(TokenKind::Semicolon);
                }

                case '!':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return TokenKind::New(TokenKind::ExclamationEquals);
                    }
                    else
                    {
                        return TokenKind::New(TokenKind::Exclamation);
                    }
                }

                case '~':
                {
                    ++it;
                    return TokenKind::New(TokenKind::Tilde);
                }


                case '(':
                {
                    ++it;
                    return TokenKind::New(TokenKind::OpenParen);
                }

                case ')':
                {
                    ++it;
                    return TokenKind::New(TokenKind::CloseParen);
                }

                case '{':
                {
                    ++it;
                    return TokenKind::New(TokenKind::OpenBrace);
                }

                case '}':
                {
                    ++it;
                    return TokenKind::New(TokenKind::CloseBrace);
                }

                case '[':
                {
                    ++it;
                    return TokenKind::New(TokenKind::OpenBracket);
                }

                case ']':
                {
                    ++it;
                    return TokenKind::New(TokenKind::CloseBracket);
                }

                default:
                {
                    return std::make_shared<const UnexpectedCharacterError>();
                }
            }
        }()));

        const TokenKindStringPair kindStringPair
        {
            kind,
            std::string{ t_context.Iterator, it },
        };

        const ScanResult scanResult
        {
            kindStringPair.String.length(),
            kindStringPair,
        };

        return Expected<Diagnosed<ScanResult, ILexerDiagnostic>, ILexerScanDiagnostic>
        {
            Diagnosed
            {
                scanResult,
                std::vector<std::shared_ptr<const ILexerDiagnostic>>{},
            }
        };
    }

    static auto ScanString(
        const ScanContext& t_context
    ) -> Diagnosed<ScanResult, ILexerDiagnostic>
    {
        std::vector<std::shared_ptr<const ILexerDiagnostic>> diagnostics{};
        auto it = t_context.Iterator;

        ACE_ASSERT(*it == '"');
        ++it;

        while (*it != '"')
        {
            if (it == t_context.IteratorEnd)
            {
                diagnostics.push_back(std::make_shared<const UnclosedStringLiteralError>(
                    Distance(t_context.Iterator, it) - 1
                ));

                break;
            }

            ++it;
        }

        if (*it == '"')
        {
            ++it;
        }

        const ScanResult scanResult
        {
            Distance(t_context.Iterator, it),
            TokenKindStringPair
            {
                TokenKind::New(TokenKind::String),
                std::string{ t_context.Iterator, it + 1 },
            },
        };

        return Diagnosed
        {
            scanResult,
            diagnostics,
        };
    }

    static auto Scan(
        const ScanContext& t_context
    ) -> Expected<Diagnosed<ScanResult, ILexerDiagnostic>, ILexerScanDiagnostic>
    {
        const auto firstCharacter = *t_context.Iterator;

        if (firstCharacter == '"')
        {
            return ScanString(t_context);
        }

        if (IsInAlphabet(firstCharacter) || (firstCharacter == '_'))
        {
            return ScanIdentifier(t_context);
        }

        if (IsNumber(firstCharacter))
        {
            return ScanNumericLiteral(t_context);
        }

        return ScanDefault(t_context);
    }

    Lexer::Lexer(
        const Compilation* const t_compilation,
        const size_t& t_fileIndex,
        const std::vector<std::string>& t_lines
    ) : m_Compilation{ t_compilation },
        m_FileIndex{ t_fileIndex },
        m_Lines{ t_lines }
    {
        ResetCharacterIterator();
    }

    auto Lexer::EatTokens() -> Diagnosed<std::vector<Token>, ILexerDiagnostic>
    {
        std::vector<std::shared_ptr<const ILexerDiagnostic>> diagnostics{};
        std::vector<Token> tokens{};

        while (const auto optDgnTokenSequence = EatTokenSequence())
        {
            diagnostics.insert(
                end(diagnostics),
                begin(optDgnTokenSequence.value().GetDiagnostics()),
                end  (optDgnTokenSequence.value().GetDiagnostics())
            );

            tokens.insert(
                end(tokens),
                begin(optDgnTokenSequence.value().Unwrap()),
                end  (optDgnTokenSequence.value().Unwrap())
            );
        }

        tokens.insert(
            end(tokens),
            Token{ std::nullopt, TokenKind::New(TokenKind::EndOfFile) }
        );

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

    auto Lexer::EatTokenSequence() -> std::optional<Diagnosed<std::vector<Token>, ILexerDiagnostic>>
    {
        std::vector<std::shared_ptr<const ILexerDiagnostic>> diagnostics{};

        EatWhitespace();

        if (IsEndOfFile())
            return std::nullopt;

        if (IsEndOfLine())
        {
            EatLine();
            return EatTokenSequence();
        }

        const auto expDgnScanResult = ScanTokenSequence();
        if (!expDgnScanResult)
        {
            diagnostics.push_back(expDgnScanResult.GetError());
            EatCharacter();

            return EatTokenSequence();
        }

        const auto& scanResult = expDgnScanResult.Unwrap().Unwrap();

        const SourceLocation sourceLocation
        {
            m_Compilation,
            m_FileIndex,
            m_LineIndex,
            Distance(m_CharacterIteratorBegin, m_CharacterIterator),
            scanResult.Length,
        };

        std::vector<Token> tokens{};
        std::transform(
            begin(scanResult.KindStringPairs),
            end  (scanResult.KindStringPairs),
            back_inserter(tokens),
            [&](const TokenKindStringPair& t_kindStringPair)
            {
                return Token
                {
                    sourceLocation,
                    t_kindStringPair.Kind,
                    t_kindStringPair.String,
                };
            }
        );

        EatCharacters(scanResult.Length);

        return Diagnosed
        {
            tokens,
            diagnostics,
        };
    }

    auto Lexer::EatCharacter() -> void
    {
        EatCharacters(1);
    }

    auto Lexer::EatCharacters(const size_t& t_count) -> void
    {
        m_CharacterIterator += t_count;
    }

    auto Lexer::EatWhitespace() -> void
    {
        while (IsWhitespace(*m_CharacterIterator))
        {
            EatCharacter();
        }
    }

    auto Lexer::EatLine() -> void
    {
        m_LineIndex++;
        ResetCharacterIterator();
    }

    auto Lexer::IsEndOfFile() const -> bool
    {
        const auto lastLineLindex = m_Lines.size() - 1;

        return 
            (m_LineIndex == lastLineLindex) &&
            IsEndOfLine();
    }

    auto Lexer::ScanTokenSequence() const -> Expected<Diagnosed<ScanResult, ILexerDiagnostic>, ILexerScanDiagnostic>
    {
        return Ace::Scan({
            m_Compilation,
            m_CharacterIterator,
            m_CharacterIteratorEnd
        });
    }

    auto Lexer::IsEndOfLine() const -> bool
    {
        return m_CharacterIterator == m_CharacterIteratorEnd;
    }

    auto Lexer::ResetCharacterIterator() -> void
    {
        const auto& line = m_Lines.at(m_LineIndex);

        m_CharacterIteratorBegin = begin(line);
        m_CharacterIterator      = begin(line);
        m_CharacterIteratorEnd   = end  (line);
    }
}
