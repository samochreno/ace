#include "C.hpp"

#include "LLVM.hpp"

namespace Ace
{
    auto C::Type::Initialize(
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

    auto C::Type::GetInt() const -> llvm::IntegerType*
    {
        return m_Int;
    }

    auto C::Type::GetChar() const -> llvm::IntegerType*
    {
        return m_Char;
    }

    auto C::Type::GetCharPointer() const -> llvm::PointerType*
    {
        return m_CharPointer;
    }

    auto C::Type::GetSize() const -> llvm::IntegerType*
    {
        return m_Size;
    }

    auto C::Type::GetVoid() const -> llvm::Type*    
    {
        return m_Void;
    }

    auto C::Type::GetVoidPointer() const -> llvm::PointerType*
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

    auto C::Function::Initialize(
        llvm::LLVMContext& t_context,
        llvm::Module& t_module,
        const Ace::C::Type& t_types
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

    auto C::Function::GetPrintf() const -> llvm::Function*
    {
        return m_Printf;
    }

    auto C::Function::GetMalloc() const -> llvm::Function*
    {
        return m_Malloc;
    }

    auto C::Function::GetCalloc() const -> llvm::Function*
    {
        return m_Calloc;
    }

    auto C::Function::GetRealloc() const -> llvm::Function*
    {
        return m_Realloc;
    }

    auto C::Function::GetFree() const -> llvm::Function*
    {
        return m_Free;
    }

    auto C::Function::GetMemset() const -> llvm::Function*
    {
        return m_Memset;
    }

    auto C::Function::GetMemcpy() const -> llvm::Function*
    {
        return m_Memcpy;
    }

    auto C::Function::GetExit() const -> llvm::Function*
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

    auto C::GetTypes() const -> const Type&
    {
        return m_Types;
    }

    auto C::GetFunctions() const -> const Function&
    {
        return m_Functions;
    }
}
