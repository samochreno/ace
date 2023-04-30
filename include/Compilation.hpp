#pragma once

#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#include "Error.hpp"
#include "Package.hpp"
#include "Natives.hpp"
#include "Scope.hpp"

namespace llvm
{
    class LLVMContext;
}

namespace Ace
{
    struct Compilation
    {
        static auto ParseAndVerify(
            const std::vector<std::string>& t_args
        ) -> Expected<std::unique_ptr<const Compilation>>;

        Package Package{};
        std::filesystem::path OutputPath{};
        std::unique_ptr<Natives> Natives{};
        std::unique_ptr<Scope> GlobalScope{};
        std::unique_ptr<llvm::LLVMContext> LLVMContext{};
    };
}

