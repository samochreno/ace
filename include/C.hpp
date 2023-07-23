#pragma once

#include "LLVM.hpp"

namespace Ace
{
    class CTypes
    {
    public:
        auto Initialize(
            llvm::LLVMContext& context,
            llvm::Module& module
        ) -> void;

        auto GetInt()     const -> llvm::IntegerType*;
        auto GetChar()    const -> llvm::IntegerType*;
        auto GetCharPtr() const -> llvm::PointerType*;
        auto GetSize()    const -> llvm::IntegerType*;
        auto GetVoid()    const -> llvm::Type*;
        auto GetVoidPtr() const -> llvm::PointerType*;

    private:
        llvm::IntegerType* m_Int{};
        llvm::IntegerType* m_Char{};
        llvm::PointerType* m_CharPtr{};
        llvm::IntegerType* m_Size{};
        llvm::Type*        m_Void{};
        llvm::PointerType* m_VoidPtr{};
    };

    class CFunctions
    {
    public:
        auto Initialize(
            llvm::LLVMContext& context,
            llvm::Module& module,
            const CTypes& types
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

    class C
    {
    public:
        auto Initialize(
            llvm::LLVMContext& context,
            llvm::Module& module
        ) -> void;

        auto GetTypes()     const -> const CTypes&;
        auto GetFunctions() const -> const CFunctions&;

    private:
        CTypes     m_Types{};
        CFunctions m_Functions{};
    };
}
