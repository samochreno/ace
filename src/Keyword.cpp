#include "Keyword.hpp"

#include <string_view>
#include <unordered_map>

#include "TokenKind.hpp"

namespace Ace
{
    auto CreateKeywordToTokenKindMap() -> std::unordered_map<std::string_view, TokenKind>
    {
        return
        {
            { std::string_view{ "__address_of" }, TokenKind::AddressOfKeyword },
            { std::string_view{    "__size_of" }, TokenKind::SizeOfKeyword    },
            { std::string_view{   "__deref_as" }, TokenKind::DerefAsKeyword   },
            { std::string_view{           "if" }, TokenKind::IfKeyword        },
            { std::string_view{         "else" }, TokenKind::ElseKeyword      },
            { std::string_view{         "elif" }, TokenKind::ElifKeyword      },
            { std::string_view{        "while" }, TokenKind::WhileKeyword     },
            { std::string_view{       "module" }, TokenKind::ModuleKeyword    },
            { std::string_view{          "ret" }, TokenKind::ReturnKeyword    },
            { std::string_view{        "trait" }, TokenKind::TraitKeyword     },
            { std::string_view{       "struct" }, TokenKind::StructKeyword    },
            { std::string_view{           "op" }, TokenKind::OpKeyword        },
            { std::string_view{          "pub" }, TokenKind::PublicKeyword    },
            { std::string_view{       "extern" }, TokenKind::ExternKeyword    },
            { std::string_view{         "cast" }, TokenKind::CastKeyword      },
            { std::string_view{         "exit" }, TokenKind::ExitKeyword      },
            { std::string_view{       "assert" }, TokenKind::AssertKeyword    },
            { std::string_view{         "impl" }, TokenKind::ImplKeyword      },
            { std::string_view{          "box" }, TokenKind::BoxKeyword       },
            { std::string_view{        "unbox" }, TokenKind::UnboxKeyword     },
            { std::string_view{         "true" }, TokenKind::TrueKeyword      },
            { std::string_view{        "false" }, TokenKind::FalseKeyword     },
            { std::string_view{          "int" }, TokenKind::IntKeyword       },
            { std::string_view{           "i8" }, TokenKind::Int8Keyword      },
            { std::string_view{          "i16" }, TokenKind::Int16Keyword     },
            { std::string_view{          "i32" }, TokenKind::Int32Keyword     },
            { std::string_view{          "i64" }, TokenKind::Int64Keyword     },
            { std::string_view{           "u8" }, TokenKind::UInt8Keyword     },
            { std::string_view{          "u16" }, TokenKind::UInt16Keyword    },
            { std::string_view{          "u32" }, TokenKind::UInt32Keyword    },
            { std::string_view{          "u64" }, TokenKind::UInt64Keyword    },
            { std::string_view{          "f32" }, TokenKind::Float32Keyword   },
            { std::string_view{          "f64" }, TokenKind::Float64Keyword   },
            { std::string_view{         "bool" }, TokenKind::BoolKeyword      },
            { std::string_view{         "void" }, TokenKind::VoidKeyword      },
        };
    }

    auto CreateTokenToKeywordKindMap() -> std::unordered_map<TokenKind, std::string_view>
    {
        std::unordered_map<TokenKind, std::string_view> map{};
        for (const auto& keywordAndTokenKindPair : CreateKeywordToTokenKindMap())
        {
            map[keywordAndTokenKindPair.second] = keywordAndTokenKindPair.first;
        }

        return map;
    }

    auto GetTokenKindNativeTypeSymbol(
        Compilation* const compilation,
        const TokenKind tokenKind
    ) -> ITypeSymbol*
    {
        switch (tokenKind)
        {
            case TokenKind::IntKeyword:
            {
                return compilation->GetNatives().Int.GetSymbol();
            }

            case TokenKind::Int8Keyword:
            {
                return compilation->GetNatives().Int8.GetSymbol();
            }

            case TokenKind::Int16Keyword:
            {
                return compilation->GetNatives().Int16.GetSymbol();
            }

            case TokenKind::Int32Keyword:
            {
                return compilation->GetNatives().Int32.GetSymbol();
            }

            case TokenKind::Int64Keyword:
            {
                return compilation->GetNatives().Int64.GetSymbol();
            }

            case TokenKind::UInt8Keyword:
            {
                return compilation->GetNatives().UInt8.GetSymbol();
            }

            case TokenKind::UInt16Keyword:
            {
                return compilation->GetNatives().UInt16.GetSymbol();
            }

            case TokenKind::UInt32Keyword:
            {
                return compilation->GetNatives().UInt32.GetSymbol();
            }

            case TokenKind::UInt64Keyword:
            {
                return compilation->GetNatives().UInt64.GetSymbol();
            }

            case TokenKind::Float32Keyword:
            {
                return compilation->GetNatives().Float32.GetSymbol();
            }

            case TokenKind::Float64Keyword:
            {
                return compilation->GetNatives().Float64.GetSymbol();
            }

            case TokenKind::BoolKeyword:
            {
                return compilation->GetNatives().Bool.GetSymbol();
            }

            case TokenKind::VoidKeyword:
            {
                return compilation->GetNatives().Void.GetSymbol();
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }
}
