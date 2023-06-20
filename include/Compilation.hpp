#pragma once

#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#include "Diagnostics.hpp"
#include "CommandLineArgumentBuffer.hpp"
#include "Package.hpp"
#include "Scope.hpp"
#include "Natives.hpp"
#include "TemplateInstantiator.hpp"

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

        CommandLineArgumentBuffer CommandLineArgumentBuffer{};
        Package Package{};
        std::filesystem::path OutputPath{};
        std::unique_ptr<Natives> Natives{};
        GlobalScope GlobalScope{};
        std::unique_ptr<TemplateInstantiator> TemplateInstantiator{};
        std::unique_ptr<llvm::LLVMContext> LLVMContext{};
    };
}
