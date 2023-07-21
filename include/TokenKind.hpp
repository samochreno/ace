#pragma once

namespace Ace
{
    enum class TokenKind
    {
        EndOfFile,

        Colon,
        ColonColon,
        Semicolon,
        Comma,
        Exclamation,
        Tilde,
        Dot,
        MinusGreaterThan,

        OpenParen,
        CloseParen,
        OpenBrace,
        CloseBrace,
        OpenBracket,
        CloseBracket,

        Identifier,

        Int8,
        Int16,
        Int32,
        Int64,
        
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        
        Int,

        Float32,
        Float64,

        String,
        Bool,

        Equals,
        EqualsEquals,
        ExclamationEquals,
        Plus,
        PlusEquals,
        Minus,
        MinusEquals,
        Asterisk,
        AsteriskEquals,
        Slash,
        SlashEquals,
        Percent,
        LessThan,
        GreaterThan,
        LessThanEquals,
        GreaterThanEquals,
        LessThanLessThan,
        GreaterThanGreaterThan,

        Caret,
        Ampersand,
        VerticalBar,
        AmpersandAmpersand,
        VerticalBarVerticalBar,
        PercentEquals,
        LessThanLessThanEquals,
        GreaterThanGreaterThanEquals,
        AmpersandEquals,
        CaretEquals,
        VerticalBarEquals,

        IfKeyword,
        ElseKeyword,
        ElifKeyword,
        WhileKeyword,
        ReturnKeyword,
        StructKeyword,
        OpKeyword,
        PublicKeyword,
        ExternKeyword,
        CastKeyword,
        ExitKeyword,
        AssertKeyword,
        ModuleKeyword,
        ImplKeyword,
        AddressOfKeyword,
        SizeOfKeyword,
        DerefAsKeyword,
        BoxKeyword,
        UnboxKeyword,
        TrueKeyword,
        FalseKeyword,
    };
}
