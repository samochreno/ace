#include "Scanner.hpp"

#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

#include "Token.hpp"
#include "Error.hpp"
#include "Keyword.hpp"
#include "Utility.hpp"
#include "NativeSymbol.hpp"

namespace Ace::Scanning
{
    namespace WhitespaceCharacter
    {
        static constexpr char Space             = ' ';
        static constexpr char FormFeed          = '\f';
        static constexpr char LineFeed          = '\n';
        static constexpr char CarriageReturn    = '\r';
        static constexpr char HorizontalTab     = '\t';
        static constexpr char VerticalTab       = '\v';
    };

    static auto CreateNativeTypeNameTokens(const NativeSymbol::Type& t_nativeType) -> std::vector<Token>
    {
        const auto name = t_nativeType.GetFullyQualifiedName();

        std::vector<Token> tokens{};

        if (name.IsGlobal)
        {
            tokens.emplace_back(Token::Kind::New(Token::Kind::ColonColon));
        }

        for (size_t i = 0; i < name.Sections.size(); i++)
        {
            if (i != 0)
            {
                tokens.emplace_back(Token::Kind::New(Token::Kind::ColonColon));
            }

            tokens.emplace_back(name.Sections.at(i).Name);
        }

        return tokens;
    }

    auto Scanner::ScanTokens(const Scanning::Kind& t_scanningKind, const std::string& t_string) -> Expected<std::vector<Token>>
    {
              auto it    = begin(t_string);
        const auto itEnd = end(t_string);

        std::vector<Token> tokens{};

        while (it != end(t_string))
        {
            ACE_TRY(newTokens, ScanToken({ t_scanningKind, it, itEnd }));
            tokens.insert(end(tokens), begin(newTokens.Value), end(newTokens.Value));
            it += newTokens.Length;
        }

        tokens.emplace_back(
            Token::Kind::New(Token::Kind::End),
            std::string{}
        );

        return tokens;
    }

    auto Scanner::ScanToken(Context t_context) -> Expected<ParseData<std::vector<Token>>>
    {
        switch (*t_context.Iterator)
        {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'm':
            case 'n':
            case 'o':
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'w':
            case 'x':
            case 'y':
            case 'z':

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':

            case '_':
            {
                return ScanIdentifier(t_context);
            }

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '0':
            {
                if (auto expScannedNumber = ScanNumber(t_context))
                {
                    return expScannedNumber;
                }
                else if (auto expScannedIdentifier = ScanIdentifier(t_context))
                {
                    return expScannedIdentifier;
                }

                ACE_TRY_UNREACHABLE();
            }

            case '#':
            {
                if (*(t_context.Iterator + 1) == ':')
                {
                    return ScanMultilineComment(t_context);
                }
                else
                {
                    return ScanComment(t_context);
                }
            }

            default:
            {
                return ScanDefault(t_context);
            }
        }
    }

    auto Scanner::ScanDefault(Context t_context) -> Expected<ParseData<std::vector<Token>>>
    {
        auto it = t_context.Iterator;
        
        ACE_TRY(tokenKind, [&]() -> Expected<Token::Kind::Set>
        {
            switch (*it)
            {
                case WhitespaceCharacter::Space:
                case WhitespaceCharacter::FormFeed:
                case WhitespaceCharacter::LineFeed:
                case WhitespaceCharacter::CarriageReturn:
                case WhitespaceCharacter::HorizontalTab:
                case WhitespaceCharacter::VerticalTab:
                {
                    ++it;
                    return Token::Kind::New();
                }

                case '#':
                {
                    ++it;

                    for (; true; ++it)
                    {
                        ACE_TRY_ASSERT(it != t_context.IteratorEnd);

                        if (*it == '#')
                            break;
                    }

                    ++it;
                    return Token::Kind::New();
                }

                case '"':
                {
                    ++it;

                    for (; true; ++it)
                    {
                        ACE_TRY_ASSERT(it != t_context.IteratorEnd);

                        ACE_TRY_ASSERT(
                            (*it != WhitespaceCharacter::FormFeed) &&
                            (*it != WhitespaceCharacter::LineFeed) &&
                            (*it != WhitespaceCharacter::CarriageReturn) &&
                            (*it != WhitespaceCharacter::VerticalTab)
                        );

                        if (*it == '"')
                            break;
                    }

                    ++it;
                    return Token::Kind::New(Token::Kind::String);
                }

                case '=':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::EqualsEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Equals);
                    }
                }

                case '+':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::PlusEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Plus);
                    }
                }

                case '-':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::MinusEquals);
                    }
                    else if (*it == '>')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::MinusGreaterThan);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Minus);
                    }
                }

                case '*':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::AsteriskEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Asterisk);
                    }
                }

                case '/':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::SlashEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Slash);
                    }
                }

                case '%':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::PercentEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Percent);
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
                            return Token::Kind::New(Token::Kind::LessThanLessThanEquals);
                        }
                        else
                        {
                            return Token::Kind::New(Token::Kind::LessThanLessThan);
                        }
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::LessThanEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::LessThan);
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
                            return Token::Kind::New(Token::Kind::GreaterThanGreaterThanEquals);
                        }
                        else
                        {
                            return Token::Kind::New(Token::Kind::GreaterThanGreaterThan);
                        }
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::GreaterThanEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::GreaterThan);
                    }
                }

                case '&':
                {
                    ++it;

                    if (*it == '&')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::AmpersandAmpersand);
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::AmpersandEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Ampersand);
                    }
                }

                case '^':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::CaretEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Caret);
                    }
                }

                case '|':
                {
                    ++it;

                    if (*it == '|')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::VerticalBarVerticalBar);
                    }
                    else if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::VerticalBarEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::VerticalBar);
                    }
                }

                case ':':
                {
                    ++it;

                    if (*it == ':')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::ColonColon);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Colon);
                    }
                }

                case '.':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::Dot);
                }

                case ',':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::Comma);
                }

                case ';':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::Semicolon);
                }

                case '!':
                {
                    ++it;

                    if (*it == '=')
                    {
                        ++it;
                        return Token::Kind::New(Token::Kind::ExclamationEquals);
                    }
                    else
                    {
                        return Token::Kind::New(Token::Kind::Exclamation);
                    }
                }

                case '~':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::Tilde);
                }


                case '(':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::OpenParen);
                }

                case ')':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::CloseParen);
                }

                case '{':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::OpenBrace);
                }

                case '}':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::CloseBrace);
                }

                case '[':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::OpenBracket);
                }

                case ']':
                {
                    ++it;
                    return Token::Kind::New(Token::Kind::CloseBracket);
                }

                default:
                {
                    ACE_TRY_UNREACHABLE();
                }
            }
        }());
            
        return ParseData
        {
            tokenKind.none() ?
                std::vector<Token>{} :
                std::vector<Token>{ { tokenKind } },
            Distance(t_context.Iterator, it)
        };
    }

    auto Scanner::ScanIdentifier(Context t_context) -> Expected<ParseData<std::vector<Token>>>
    {
        auto it = t_context.Iterator;

        if (*it == '$')
        {
            ACE_TRY_ASSERT(t_context.ScanningKind == Scanning::Kind::Metadata);
            ++it;
        }

        for (; it != t_context.IteratorEnd; ++it)
        {
            if (
                !IsInAlphabet(*it) &&
                !IsNumber(*it) &&
                !(*it == '_')
                )
                break;
        }

        const auto tokens = [&]() -> std::vector<Token>
        {
            const std::string value{ t_context.Iterator, it };

            if      (value == Keyword::If)          return std::vector{ Token{ Token::Kind::New(Token::Kind::IfKeyword) } };
            else if (value == Keyword::Else)        return std::vector{ Token{ Token::Kind::New(Token::Kind::ElseKeyword) } };
            else if (value == Keyword::Elif)        return std::vector{ Token{ Token::Kind::New(Token::Kind::ElifKeyword) } };
            else if (value == Keyword::While)       return std::vector{ Token{ Token::Kind::New(Token::Kind::WhileKeyword) } };
            else if (value == Keyword::Return)      return std::vector{ Token{ Token::Kind::New(Token::Kind::ReturnKeyword) } };
            else if (value == Keyword::Struct)      return std::vector{ Token{ Token::Kind::New(Token::Kind::StructKeyword) } };
            else if (value == Keyword::Operator)    return std::vector{ Token{ Token::Kind::New(Token::Kind::OperatorKeyword) } };
            else if (value == Keyword::Public)      return std::vector{ Token{ Token::Kind::New(Token::Kind::PublicKeyword) } };
            else if (value == Keyword::Extern)      return std::vector{ Token{ Token::Kind::New(Token::Kind::ExternKeyword) } };
            else if (value == Keyword::Cast)        return std::vector{ Token{ Token::Kind::New(Token::Kind::CastKeyword) } };
            else if (value == Keyword::Exit)        return std::vector{ Token{ Token::Kind::New(Token::Kind::ExitKeyword) } };
            else if (value == Keyword::Assert)      return std::vector{ Token{ Token::Kind::New(Token::Kind::AssertKeyword) } };
            else if (value == Keyword::Module)      return std::vector{ Token{ Token::Kind::New(Token::Kind::ModuleKeyword) } };
            else if (value == Keyword::Impl)        return std::vector{ Token{ Token::Kind::New(Token::Kind::ImplKeyword) } };
            else if (value == Keyword::Expl)        return std::vector{ Token{ Token::Kind::New(Token::Kind::ExplKeyword) } };
            else if (value == Keyword::AddressOf)   return std::vector{ Token{ Token::Kind::New(Token::Kind::AddressOfKeyword) } };
            else if (value == Keyword::SizeOf)      return std::vector{ Token{ Token::Kind::New(Token::Kind::SizeOfKeyword) } };
            else if (value == Keyword::DerefAs)     return std::vector{ Token{ Token::Kind::New(Token::Kind::DerefAsKeyword) } };
            else if (value == Keyword::Box)         return std::vector{ Token{ Token::Kind::New(Token::Kind::BoxKeyword) } };
            else if (value == Keyword::Unbox)       return std::vector{ Token{ Token::Kind::New(Token::Kind::UnboxKeyword) } };
            else if (value == Keyword::True)        return std::vector{ Token{ Token::Kind::New(Token::Kind::TrueKeyword) } };
            else if (value == Keyword::False)       return std::vector{ Token{ Token::Kind::New(Token::Kind::FalseKeyword) } };

            else if (value == Keyword::Int8)        return CreateNativeTypeNameTokens(NativeSymbol::Int8);
            else if (value == Keyword::Int16)       return CreateNativeTypeNameTokens(NativeSymbol::Int16);
            else if (value == Keyword::Int32)       return CreateNativeTypeNameTokens(NativeSymbol::Int32);
            else if (value == Keyword::Int64)       return CreateNativeTypeNameTokens(NativeSymbol::Int64);
            else if (value == Keyword::UInt8)       return CreateNativeTypeNameTokens(NativeSymbol::UInt8);
            else if (value == Keyword::UInt16)      return CreateNativeTypeNameTokens(NativeSymbol::UInt16);
            else if (value == Keyword::UInt32)      return CreateNativeTypeNameTokens(NativeSymbol::UInt32);
            else if (value == Keyword::UInt64)      return CreateNativeTypeNameTokens(NativeSymbol::UInt64);
            else if (value == Keyword::Int)         return CreateNativeTypeNameTokens(NativeSymbol::Int);
            else if (value == Keyword::Float32)     return CreateNativeTypeNameTokens(NativeSymbol::Float32);
            else if (value == Keyword::Float64)     return CreateNativeTypeNameTokens(NativeSymbol::Float64);
            else if (value == Keyword::Bool)        return CreateNativeTypeNameTokens(NativeSymbol::Bool);
            else if (value == Keyword::Void)        return CreateNativeTypeNameTokens(NativeSymbol::Void);

            return std::vector{ Token{ Token::Kind::New(Token::Kind::Identifier), value } };
        }();

        return ParseData
        {
            tokens,
            Distance(t_context.Iterator, it)
        };
    }

    static auto GetNumberType(
        const std::string& t_numberString,
        const std::string& t_typeString,
        const std::string& t_typeSizeString,
        const bool& t_hasDecimalPoint
    ) -> Expected<Token::Kind::Set>
    {
        switch (t_typeString.front())
        {
            case 'i':
            {
                ACE_TRY_ASSERT(!t_hasDecimalPoint);
                ACE_TRY(value, [&]() -> Expected<int64_t>
                {
                    try
                    {
                        return std::stoll(t_numberString);
                    }
                    catch (const std::out_of_range&)
                    {
                        ACE_TRY_UNREACHABLE();
                    }
                }());

                if (t_typeSizeString == "8")
                {
                    ACE_TRY_ASSERT(
                        (value >= std::numeric_limits<int8_t>::min()) &&
                        (value <= std::numeric_limits<int8_t>::max())
                    );

                    return Token::Kind::New(Token::Kind::Int8);
                }
                else if (t_typeSizeString == "16")
                {
                    ACE_TRY_ASSERT(
                        (value >= std::numeric_limits<int16_t>::min()) &&
                        (value <= std::numeric_limits<int16_t>::max())
                    );

                    return Token::Kind::New(Token::Kind::Int16);
                }
                else if (t_typeSizeString == "32")
                {
                    ACE_TRY_ASSERT(
                        (value >= std::numeric_limits<int32_t>::min()) &&
                        (value <= std::numeric_limits<int32_t>::max())
                    );

                    return Token::Kind::New(Token::Kind::Int32);
                }
                else if (t_typeSizeString == "64")
                {
                    ACE_TRY_ASSERT(
                        (value >= std::numeric_limits<int64_t>::min()) &&
                        (value <= std::numeric_limits<int64_t>::max())
                    );

                    return Token::Kind::New(Token::Kind::Int64);
                }
            }

            case 'u':
            {
                ACE_TRY_ASSERT(!t_hasDecimalPoint);
                ACE_TRY(value, [&]() -> Expected<uint64_t>
                {
                    try
                    {
                        return std::stoull(t_numberString);
                    }
                    catch (const std::out_of_range&)
                    {
                        ACE_TRY_UNREACHABLE();
                    }
                }());

                if (t_typeSizeString == "8")
                {
                    ACE_TRY_ASSERT(
                        (value >= std::numeric_limits<uint8_t>::min()) &&
                        (value <= std::numeric_limits<uint8_t>::max())
                    );

                    return Token::Kind::New(Token::Kind::UInt8);
                }
                else if (t_typeSizeString == "16")
                {
                    ACE_TRY_ASSERT(
                        (value >= std::numeric_limits<uint16_t>::min()) &&
                        (value <= std::numeric_limits<uint16_t>::max())
                    );

                    return Token::Kind::New(Token::Kind::UInt16);
                }
                else if (t_typeSizeString == "32")
                {
                    ACE_TRY_ASSERT(
                        (value >= std::numeric_limits<uint32_t>::min()) &&
                        (value <= std::numeric_limits<uint32_t>::max())
                    );

                    return Token::Kind::New(Token::Kind::UInt32);
                }
                else if (t_typeSizeString == "64")
                {
                    ACE_TRY_ASSERT(
                        (value >= std::numeric_limits<uint64_t>::min()) &&
                        (value <= std::numeric_limits<uint64_t>::max())
                    );

                    return Token::Kind::New(Token::Kind::UInt64);
                }
            }

            case 'f':
            {
                double value{};
                try
                {
                    value = std::stod(t_numberString);
                }
                catch (const std::out_of_range&)
                {
                    ACE_TRY_UNREACHABLE();
                }

                if (t_typeSizeString == "32")
                {
                    return Token::Kind::New(Token::Kind::Float32);
                }
                else if (t_typeSizeString == "64")
                {
                    return Token::Kind::New(Token::Kind::Float64);
                }
            }
        }
        
        ACE_TRY_UNREACHABLE();
    }

    auto Scanner::ScanNumber(Context t_context) -> Expected<ParseData<std::vector<Token>>>
    {
        auto it = t_context.Iterator;

        std::string numberString{};

        ACE_TRY_ASSERT(IsNumber(*it));

        bool hasDecimalPoint = false;
        for (; true; [&](){ numberString += *it; ++it; }())
        {
            if (IsNumber(*it))
                continue;

            if (*it == '.')
            {
                ACE_TRY_ASSERT(IsNumber(*(it - 1)));
                ACE_TRY_ASSERT(!hasDecimalPoint);
                continue;
            }

            if (*it == '_')
            {
                ACE_TRY_ASSERT(IsNumber(*(it - 1)));
                continue;
            }

            break;
        }

        ACE_TRY_ASSERT(IsNumber(numberString.back()));

        if (!IsInAlphabet(*it))
        {
            ACE_TRY_ASSERT(!hasDecimalPoint);
            ACE_TRY(value, [&]() -> Expected<int64_t>
            {
                try
                {
                    return std::stoll(numberString);
                }
                catch (const std::out_of_range&)
                {
                    ACE_TRY_UNREACHABLE();
                }
            }());

            return ParseData
            {
                std::vector{ Token{ Token::Kind::New(Token::Kind::Int), numberString } },
                Distance(t_context.Iterator, it)
            };
        }

        std::string typeString{};
        typeString += *it;
        ++it;

        for (; true; [&]() { typeString += *it; ++it; }())
        {
            if (!IsNumber(*it))
                break;
        }

        const std::string typeSizeString{ begin(typeString) + 1, end(typeString) };

        ACE_TRY(tokenKind, [&]() -> Expected<Token::Kind::Set>
        {
            return GetNumberType(
                numberString,
                typeString,
                typeSizeString,
                hasDecimalPoint
            );
        }());

        return ParseData
        {
            std::vector{ Token{ tokenKind, numberString } },
            Distance(t_context.Iterator, it)
        };
    }

    auto Scanner::ScanComment(Context t_context) -> Expected<ParseData<std::vector<Token>>>
    {
        auto it = t_context.Iterator;
        
        ACE_TRY_ASSERT(*it == '#');
        ++it;

        for (; it != t_context.IteratorEnd; ++it)
        {
            if (
                (*it == WhitespaceCharacter::FormFeed) ||
                (*it == WhitespaceCharacter::LineFeed) ||
                (*it == WhitespaceCharacter::CarriageReturn) ||
                (*it == WhitespaceCharacter::VerticalTab)
                )
                break;
        }

        return ParseData
        {
            std::vector<Token>{},
            Distance(t_context.Iterator, it)
        };
    }

    auto Scanner::ScanMultilineComment(Context t_context) -> Expected<ParseData<std::vector<Token>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(*it == '#');
        ++it;

        ACE_TRY_ASSERT(*it == ':');
        ++it;

        size_t indentLevel = 1;
        char previousCharacter = 0;
        for (;; [&](){ previousCharacter = *it; ++it; }())
        {
            ACE_TRY_ASSERT(it != t_context.IteratorEnd);

            switch (previousCharacter)
            {
                case ':':
                {
                    if (*it == '#')
                        indentLevel--;

                    if (indentLevel == 0)
                    {
                        ++it;
                        return ParseData
                        {
                            std::vector<Token>{},
                            Distance(t_context.Iterator, it)
                        };
                    }

                    break;
                }

                case '#':
                {
                    if (*it == ':')
                        indentLevel++;

                    break;
                }
            }
        }

        ACE_TRY_UNREACHABLE();
    }
}
