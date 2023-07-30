#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <filesystem>

#include "Diagnostic.hpp"
#include "SrcBuffer.hpp"
#include "CLIArgBuffer.hpp"
#include "FileBuffer.hpp"
#include "Package.hpp"
#include "Scope.hpp"
#include "Natives.hpp"
#include "TemplateInstantiator.hpp"
#include "GlobalDiagnosticBag.hpp"

namespace llvm
{
    class LLVMContext;
}

namespace Ace
{
    class ITypeSymbol;

    struct Compilation
    {
        static auto Parse(
            std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
            const std::vector<std::string_view>& args
        ) -> Expected<std::unique_ptr<const Compilation>>;

        const CLIArgBuffer* CLIArgBuffer{};
        const FileBuffer* PackageFileBuffer{};
        Package Package{};
        std::filesystem::path OutputPath{};
        std::unique_ptr<Natives> Natives{};
        GlobalScope GlobalScope{};
        std::unique_ptr<TemplateInstantiator> TemplateInstantiator{};
        std::unique_ptr<llvm::LLVMContext> LLVMContext{};
        std::unique_ptr<GlobalDiagnosticBag> Diagnostics{};
        ITypeSymbol* ErrorTypeSymbol{};
    };
}
