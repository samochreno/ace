#include "C.hpp"

#include "LLVM.hpp"

namespace Ace
{
    auto CTypes::Initialize(
        llvm::LLVMContext& context,
        llvm::Module& module
    ) -> void
    {
        constexpr unsigned int intBitCount = sizeof(int) * CHAR_BIT;
        constexpr unsigned int charBitCount = sizeof(char) * CHAR_BIT;
        constexpr unsigned int sizeBitCount = sizeof(size_t) * CHAR_BIT;

        m_Int     = llvm::IntegerType::get(context, intBitCount);
        m_Char    = llvm::IntegerType::get(context, charBitCount);
        m_CharPtr = llvm::PointerType::get(m_Char, 0);
        m_Size    = llvm::IntegerType::get(context, sizeBitCount);
        m_Void    = llvm::Type::getVoidTy(context);
        m_VoidPtr = llvm::Type::getInt8PtrTy(context);
    }

    auto CTypes::GetInt() const -> llvm::IntegerType*
    {
        return m_Int;
    }

    auto CTypes::GetChar() const -> llvm::IntegerType*
    {
        return m_Char;
    }

    auto CTypes::GetCharPtr() const -> llvm::PointerType*
    {
        return m_CharPtr;
    }

    auto CTypes::GetSize() const -> llvm::IntegerType*
    {
        return m_Size;
    }

    auto CTypes::GetVoid() const -> llvm::Type*    
    {
        return m_Void;
    }

    auto CTypes::GetVoidPtr() const -> llvm::PointerType*
    {
        return m_VoidPtr;
    }

    static auto LoadCFunction(
        llvm::Module& module,
        llvm::Function** const field,
        const char* const name,
        llvm::FunctionType* const type
    ) -> void
    {
        auto functionCallee = module.getOrInsertFunction(
            name,
            type
        );

        auto* const function = llvm::cast<llvm::Function>(
            functionCallee.getCallee()
        );
        *field = function;
    }

    auto CFunctions::Initialize(
        llvm::LLVMContext& context,
        llvm::Module& module,
        const Ace::CTypes& types
    ) -> void
    {
        constexpr unsigned int intBitCount = sizeof(int) * CHAR_BIT;
        constexpr unsigned int charBitCount = sizeof(char) * CHAR_BIT;
        constexpr unsigned int sizeBitCount = sizeof(size_t) * CHAR_BIT;

        auto* const intType     = llvm::IntegerType::get(context, intBitCount);
        auto* const charType    = llvm::IntegerType::get(context, charBitCount);
        auto* const charPtrType = llvm::PointerType::get(charType, 0);
        auto* const sizeType    = llvm::IntegerType::get(context, sizeBitCount);
        auto* const voidType    = llvm::Type::getVoidTy(context);
        auto* const voidPtrType = llvm::Type::getInt8PtrTy(context);

        {
            auto* const type = llvm::FunctionType::get(
                intType,
                { charPtrType },
                true
            );

            LoadCFunction(module, &m_Printf, "printf", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { sizeType },
                false
            );

            LoadCFunction(module, &m_Malloc, "malloc", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { sizeType, sizeType },
                false
            );

            LoadCFunction(module, &m_Calloc, "calloc", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { voidPtrType, sizeType },
                false
            );

            LoadCFunction(module, &m_Realloc, "realloc", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { voidPtrType },
                false
            );

            LoadCFunction(module, &m_Free, "free", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { voidPtrType, intType, sizeType },
                false
            );

            LoadCFunction(module, &m_Memset, "memset", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { voidPtrType, voidPtrType, sizeType },
                false
            );

            LoadCFunction(module, &m_Memcpy, "memcpy", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidType,
                { intType },
                false
            );

            LoadCFunction(module, &m_Exit, "exit", type);
        }
    }

    auto CFunctions::GetPrintf() const -> llvm::Function*
    {
        return m_Printf;
    }

    auto CFunctions::GetMalloc() const -> llvm::Function*
    {
        return m_Malloc;
    }

    auto CFunctions::GetCalloc() const -> llvm::Function*
    {
        return m_Calloc;
    }

    auto CFunctions::GetRealloc() const -> llvm::Function*
    {
        return m_Realloc;
    }

    auto CFunctions::GetFree() const -> llvm::Function*
    {
        return m_Free;
    }

    auto CFunctions::GetMemset() const -> llvm::Function*
    {
        return m_Memset;
    }

    auto CFunctions::GetMemcpy() const -> llvm::Function*
    {
        return m_Memcpy;
    }

    auto CFunctions::GetExit() const -> llvm::Function*
    {
        return m_Exit;
    }

    auto C::Initialize(
        llvm::LLVMContext& context,
        llvm::Module& module
    ) -> void
    {
        m_Types.Initialize(context, module);
        m_Functions.Initialize(context, module, m_Types);
    }

    auto C::GetTypes() const -> const CTypes&
    {
        return m_Types;
    }

    auto C::GetFunctions() const -> const CFunctions&
    {
        return m_Functions;
    }
}
