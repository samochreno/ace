#include "C.hpp"

#include "LLVM.hpp"

namespace Ace
{
    auto CTypes::Initialize(
        llvm::LLVMContext& t_context,
        llvm::Module& t_module
    ) -> void
    {
        constexpr unsigned int intBitCount = sizeof(int) * CHAR_BIT;
        constexpr unsigned int charBitCount = sizeof(char) * CHAR_BIT;
        constexpr unsigned int sizeBitCount = sizeof(size_t) * CHAR_BIT;

        m_Int           = llvm::IntegerType::get(t_context, intBitCount);
        m_Char          = llvm::IntegerType::get(t_context, charBitCount);
        m_CharPointer   = llvm::PointerType::get(m_Char, 0);
        m_Size          = llvm::IntegerType::get(t_context, sizeBitCount);
        m_Void          = llvm::Type::getVoidTy(t_context);
        m_VoidPointer   = llvm::Type::getInt8PtrTy(t_context);
    }

    auto CTypes::GetInt() const -> llvm::IntegerType*
    {
        return m_Int;
    }

    auto CTypes::GetChar() const -> llvm::IntegerType*
    {
        return m_Char;
    }

    auto CTypes::GetCharPointer() const -> llvm::PointerType*
    {
        return m_CharPointer;
    }

    auto CTypes::GetSize() const -> llvm::IntegerType*
    {
        return m_Size;
    }

    auto CTypes::GetVoid() const -> llvm::Type*    
    {
        return m_Void;
    }

    auto CTypes::GetVoidPointer() const -> llvm::PointerType*
    {
        return m_VoidPointer;
    }

    static auto LoadCFunction(
        llvm::Module& t_module,
        llvm::Function** const t_field,
        const char* const t_name,
        llvm::FunctionType* const t_type
    ) -> void
    {
        auto functionCallee = t_module.getOrInsertFunction(
            t_name,
            t_type
        );

        auto* const function = llvm::cast<llvm::Function>(
            functionCallee.getCallee()
        );
        *t_field = function;
    }

    auto CFunctions::Initialize(
        llvm::LLVMContext& t_context,
        llvm::Module& t_module,
        const Ace::CTypes& t_types
    ) -> void
    {
        constexpr unsigned int intBitCount = sizeof(int) * CHAR_BIT;
        constexpr unsigned int charBitCount = sizeof(char) * CHAR_BIT;
        constexpr unsigned int sizeBitCount = sizeof(size_t) * CHAR_BIT;

        auto* const intType     = llvm::IntegerType::get(t_context, intBitCount);
        auto* const charType    = llvm::IntegerType::get(t_context, charBitCount);
        auto* const charPtrType = llvm::PointerType::get(charType, 0);
        auto* const sizeType    = llvm::IntegerType::get(t_context, sizeBitCount);
        auto* const voidType    = llvm::Type::getVoidTy(t_context);
        auto* const voidPtrType = llvm::Type::getInt8PtrTy(t_context);

        {
            auto* const type = llvm::FunctionType::get(
                intType,
                { charPtrType },
                true
            );

            LoadCFunction(t_module, &m_Printf, "printf", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { sizeType },
                false
            );

            LoadCFunction(t_module, &m_Malloc, "malloc", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { sizeType, sizeType },
                false
            );

            LoadCFunction(t_module, &m_Calloc, "calloc", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { voidPtrType, sizeType },
                false
            );

            LoadCFunction(t_module, &m_Realloc, "realloc", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { voidPtrType },
                false
            );

            LoadCFunction(t_module, &m_Free, "free", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { voidPtrType, intType, sizeType },
                false
            );

            LoadCFunction(t_module, &m_Memset, "memset", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidPtrType,
                { voidPtrType, voidPtrType, sizeType },
                false
            );

            LoadCFunction(t_module, &m_Memcpy, "memcpy", type);
        }

        {
            auto* const type = llvm::FunctionType::get(
                voidType,
                { intType },
                false
            );

            LoadCFunction(t_module, &m_Exit, "exit", type);
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
        llvm::LLVMContext& t_context,
        llvm::Module& t_module
    ) -> void
    {
        m_Types.Initialize(t_context, t_module);
        m_Functions.Initialize(t_context, t_module, m_Types);
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
