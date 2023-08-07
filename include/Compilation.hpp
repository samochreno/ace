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
#include "ErrorSymbols.hpp"

namespace llvm
{
    class LLVMContext;
}

namespace Ace
{
    class Compilation
    {
    public:
        static auto Parse(
            std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
            const std::vector<std::string_view>& args
        ) -> Expected<std::unique_ptr<Compilation>>;

        auto GetCLIArgBuffer() const -> const CLIArgBuffer*;
        auto GetPackageFileBuffer() const -> const FileBuffer*;
        auto GetPackage() const -> const Package&;
        auto GetOutputPath() const -> const std::filesystem::path&;
        auto GetNatives() const -> const Natives&;
        auto GetGlobalScope() const -> const std::shared_ptr<Scope>&;
        auto GetTemplateInstantiator() const -> const TemplateInstantiator&;
        auto GetTemplateInstantiator()       ->       TemplateInstantiator&;
        auto GetLLVMContext() const -> const llvm::LLVMContext&;
        auto GetLLVMContext()       ->       llvm::LLVMContext&;
        auto GetErrorSymbols() const -> const ErrorSymbols&;

    private:
        const CLIArgBuffer* m_CLIArgBuffer{};
        const FileBuffer* m_PackageFileBuffer{};
        Package m_Package{};
        std::filesystem::path m_OutputPath{};
        std::unique_ptr<const Natives> m_Natives{};
        GlobalScope m_GlobalScope{};
        TemplateInstantiator m_TemplateInstantiator{};
        llvm::LLVMContext m_LLVMContext{};
        ErrorSymbols m_ErrorSymbols{};
    };
}
