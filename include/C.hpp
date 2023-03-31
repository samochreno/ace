#pragma once

#include "LLVM.hpp"

namespace Ace
{
    struct C
    {
        class Type
        {
        public:
            auto Initialize(llvm::LLVMContext& t_context, llvm::Module& t_module) -> void;

            auto GetInt()           const -> llvm::IntegerType* { return m_Int; }
            auto GetChar()          const -> llvm::IntegerType* { return m_Char; }
            auto GetCharPointer()   const -> llvm::PointerType* { return m_CharPointer; }
            auto GetSize()          const -> llvm::IntegerType* { return m_Size; }
            auto GetVoid()          const -> llvm::Type*        { return m_Void; }
            auto GetVoidPointer()   const -> llvm::PointerType* { return m_VoidPointer; }

        private:
            llvm::IntegerType*  m_Int{};
            llvm::IntegerType*  m_Char{};
            llvm::PointerType*  m_CharPointer{};
            llvm::IntegerType*  m_Size{};
            llvm::Type*         m_Void{};
            llvm::PointerType*  m_VoidPointer{};
        };

        class Function
        {
        public:
            auto Initialize(llvm::LLVMContext& t_context, llvm::Module& t_module, const C::Type& t_types) -> void;

            auto GetPrintf()    const -> llvm::Function* { return m_Printf; }
            auto GetMalloc()    const -> llvm::Function* { return m_Malloc; }
            auto GetCalloc()    const -> llvm::Function* { return m_Calloc; }
            auto GetRealloc()   const -> llvm::Function* { return m_Realloc; }
            auto GetFree()      const -> llvm::Function* { return m_Free; }
            auto GetMemset()    const -> llvm::Function* { return m_Memset; }
            auto GetMemcpy()    const -> llvm::Function* { return m_Memcpy; }
            auto GetExit()      const -> llvm::Function* { return m_Exit; }

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

        auto Initialize(llvm::LLVMContext& t_context, llvm::Module& t_module) -> void;

        auto GetTypes()     const -> const Type&        { return m_Types; }
        auto GetFunctions() const -> const Function&    { return m_Functions; }

    private:
        Type        m_Types{};
        Function    m_Functions{};
    };
}
