#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <filesystem>

#include "Diagnostics.hpp"
#include "CommandLineArgumentBuffer.hpp"
#include "FileBuffer.hpp"
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
        static auto Parse(
            const std::vector<std::string_view>& t_args
        ) -> Expected<Diagnosed<std::unique_ptr<const Compilation>>>;

        CommandLineArgumentBuffer CommandLineArgumentBuffer{};
        FileBuffer PackageFileBuffer{};
        Package Package{};
        std::filesystem::path OutputPath{};
        std::unique_ptr<Natives> Natives{};
        GlobalScope GlobalScope{};
        std::unique_ptr<TemplateInstantiator> TemplateInstantiator{};
        std::unique_ptr<llvm::LLVMContext> LLVMContext{};
    };
}
