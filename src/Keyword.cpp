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
            {    "__address_of", TokenKind::AddressOfKeyword   },
            {       "__size_of", TokenKind::SizeOfKeyword      },
            {      "__deref_as", TokenKind::DerefAsKeyword     },
            {          "__copy", TokenKind::CopyKeyword        },
            {          "__drop", TokenKind::DropKeyword        },
            { "__type_info_ptr", TokenKind::TypeInfoPtrKeyword },
            {      "__vtbl_ptr", TokenKind::VtblPtrKeyword     },
            {              "if", TokenKind::IfKeyword          },
            {            "else", TokenKind::ElseKeyword        },
            {            "elif", TokenKind::ElifKeyword        },
            {           "while", TokenKind::WhileKeyword       },
            {            "self", TokenKind::SelfKeyword        },
            {             "ret", TokenKind::RetKeyword         },
            {             "mod", TokenKind::ModKeyword         },
            {           "trait", TokenKind::TraitKeyword       },
            {          "struct", TokenKind::StructKeyword      },
            {              "op", TokenKind::OpKeyword          },
            {             "pub", TokenKind::PubKeyword         },
            {          "extern", TokenKind::ExternKeyword      },
            {            "cast", TokenKind::CastKeyword        },
            {            "exit", TokenKind::ExitKeyword        },
            {          "assert", TokenKind::AssertKeyword      },
            {            "impl", TokenKind::ImplKeyword        },
            {             "for", TokenKind::ForKeyword         },
            {            "lock", TokenKind::LockKeyword        },
            {             "box", TokenKind::BoxKeyword         },
            {           "unbox", TokenKind::UnboxKeyword       },
            {             "use", TokenKind::UseKeyword         },
            {           "where", TokenKind::WhereKeyword       },
            {            "true", TokenKind::TrueKeyword        },
            {           "false", TokenKind::FalseKeyword       },
            {            "Self", TokenKind::SelfTypeKeyword    },
            {             "int", TokenKind::IntKeyword         },
            {              "i8", TokenKind::Int8Keyword        },
            {             "i16", TokenKind::Int16Keyword       },
            {             "i32", TokenKind::Int32Keyword       },
            {             "i64", TokenKind::Int64Keyword       },
            {              "u8", TokenKind::UInt8Keyword       },
            {             "u16", TokenKind::UInt16Keyword      },
            {             "u32", TokenKind::UInt32Keyword      },
            {             "u64", TokenKind::UInt64Keyword      },
            {             "f32", TokenKind::Float32Keyword     },
            {             "f64", TokenKind::Float64Keyword     },
            {            "bool", TokenKind::BoolKeyword        },
            {            "void", TokenKind::VoidKeyword        },
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
                return compilation->GetVoidTypeSymbol();
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }
}
