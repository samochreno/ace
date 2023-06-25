#pragma once

#include "LLVM.hpp"

namespace Ace
{
    class CTypes
    {
    public:
        auto Initialize(
            llvm::LLVMContext& t_context,
            llvm::Module& t_module
        ) -> void;

        auto GetInt()         const -> llvm::IntegerType*;
        auto GetChar()        const -> llvm::IntegerType*;
        auto GetCharPointer() const -> llvm::PointerType*;
        auto GetSize()        const -> llvm::IntegerType*;
        auto GetVoid()        const -> llvm::Type*;
        auto GetVoidPointer() const -> llvm::PointerType*;

    private:
        llvm::IntegerType* m_Int{};
        llvm::IntegerType* m_Char{};
        llvm::PointerType* m_CharPointer{};
        llvm::IntegerType* m_Size{};
        llvm::Type*        m_Void{};
        llvm::PointerType* m_VoidPointer{};
    };

    class CFunctions
    {
    public:
        auto Initialize(
            llvm::LLVMContext& t_context,
            llvm::Module& t_module,
            const CTypes& t_types
        ) -> void;

        auto GetPrintf()  const -> llvm::Function*;
        auto GetMalloc()  const -> llvm::Function*;
        auto GetCalloc()  const -> llvm::Function*;
        auto GetRealloc() const -> llvm::Function*;
        auto GetFree()    const -> llvm::Function*;
        auto GetMemset()  const -> llvm::Function*;
        auto GetMemcpy()  const -> llvm::Function*;
        auto GetExit()    const -> llvm::Function*;

    private:
        llvm::Function* m_Printf{};
        llvm::Function* m_Malloc{};
        llvm::Function* m_Calloc{};
        llvm::Function* m_Realloc{};
        llvm::Function* m_Free{};
        llvm::Function* m_Memset{};
        llvm::Function* m_Memcpy{};
        llvm::Function* m_Exit{};
    };

    struct C
    {
        auto Initialize(
            llvm::LLVMContext& t_context,
            llvm::Module& t_module
        ) -> void;

        auto GetTypes()     const -> const CTypes&;
        auto GetFunctions() const -> const CFunctions&;

    private:
        CTypes     m_Types{};
        CFunctions m_Functions{};
    };
}
