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
#include "ErrorSymbols.hpp"

namespace Ace
{
    class ITypeSymbol;

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

        auto GetGlobalScope() const -> const std::shared_ptr<Scope>&;
        auto GetPackageBodyScope() const -> const std::shared_ptr<Scope>&;

        auto GetVoidTypeSymbol() const -> ITypeSymbol*;
        auto GetErrorSymbols() const -> const ErrorSymbols&;
        auto GetNatives() const -> const Natives&;

    private:
        const CLIArgBuffer* m_CLIArgBuffer{};

        const FileBuffer* m_PackageFileBuffer{};
        Package m_Package{};
        std::filesystem::path m_OutputPath{};

        GlobalScope m_GlobalScope{};
        std::shared_ptr<Scope> m_PackageBodyScope{};

        ITypeSymbol* m_VoidTypeSymbol{};
        ErrorSymbols m_ErrorSymbols{};
        std::unique_ptr<const Natives> m_Natives{};
    };
}
