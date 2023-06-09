#include "Compilation.hpp"

#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <llvm/IR/LLVMContext.h>

#include "Diagnostics.hpp"
#include "Package.hpp"
#include "Natives.hpp"
#include "Scope.hpp"
#include "TemplateInstantiator.hpp"

namespace Ace
{
    struct CompilationOptions
    {
        std::string PackagePath{};
        std::string OutputPath{};
    };

    struct ArgumentNameValuePair
    {
        std::string Name{};
        std::optional<std::string> OptValue{};
    };

    enum class ArgumentKind
    {
        None,
        Short,
        Long,
    };

    static auto ParseArgument(
        const std::string& t_arg,
        const ArgumentKind& t_kind
    ) -> Expected<ArgumentNameValuePair>
    {
        ACE_ASSERT(
            (t_kind == ArgumentKind::Short) ||
            (t_kind == ArgumentKind::Long)
        );

        const std::string startStr = 
            (t_kind == ArgumentKind::Short) ? "-" : "--";

        ACE_TRY_ASSERT(t_arg.starts_with(startStr));

        const size_t equalsPos = t_arg.find('=');
        if (equalsPos == std::string::npos)
        {
            const std::string name
            { 
                begin(t_arg) + startStr.size(), 
                end  (t_arg)
            };
            ACE_TRY_ASSERT(!name.empty());

            return ArgumentNameValuePair
            {
                name,
                std::nullopt,
            };
        }
        else
        {
            const std::string name
            { 
                begin(t_arg) + startStr.size(), 
                begin(t_arg) + equalsPos
            };
            ACE_TRY_ASSERT(!name.empty());

            const std::string value
            {
                begin(t_arg) + equalsPos + 1,
                end  (t_arg)
            };
            ACE_TRY_ASSERT(!value.empty());

            return ArgumentNameValuePair
            {
                name,
                value
            };
        }
    }

    static auto ParseArgument(
        const std::string& t_arg
    ) -> Expected<ArgumentNameValuePair>
    {
        ACE_ASSERT(t_arg.starts_with('-'));

        ACE_TRY_ASSERT(t_arg.size() > 1);

        const bool hasSecondDash = t_arg.at(1) == '-';
        return ParseArgument(
            t_arg, 
            hasSecondDash ? ArgumentKind::Long : ArgumentKind::Short
        );
    }

    static auto ParseOptions(
        const std::vector<std::string>& t_args
    ) -> Expected<CompilationOptions>
    {
        std::vector<std::string> packagePaths{};
        std::unordered_map<std::string, std::optional<std::string>> argumentMap{};

        ACE_TRY_VOID(TransformExpectedVector(t_args,
        [&](const std::string& t_arg) -> Expected<void>
        {
            if (t_arg.starts_with('-'))
            {
                ACE_TRY(argNameValuePair, ParseArgument(t_arg));
                argumentMap[argNameValuePair.Name] = argNameValuePair.OptValue;
            }
            else
            {
                packagePaths.push_back(t_arg);
            }

            return ExpectedVoid;
        }));

        ACE_TRY_ASSERT(packagePaths.size() == 1);
        const std::string& packagePath = packagePaths.front();

        const auto outputPath = [&]() -> std::filesystem::path
        {
            const auto foundOutputPathArgumentIt = argumentMap.find("o");

            if (foundOutputPathArgumentIt == end(argumentMap))
            {
                return { "./" };
            }

            const auto& optOutputPath = foundOutputPathArgumentIt->second;
            ACE_ASSERT(optOutputPath.has_value());
            return optOutputPath.value();
        }();

        return CompilationOptions
        {
            std::filesystem::path{ packagePath },
            std::filesystem::path{ outputPath },
        };
    }

    auto Compilation::ParseAndVerify(
        const std::vector<std::string>& t_args
    ) -> Expected<std::unique_ptr<const Compilation>>
    {
        ACE_TRY(options, ParseOptions(t_args));
        ACE_TRY(package, Package::New(options.PackagePath));

        const auto& outputPath = options.OutputPath;

        if (
            !std::filesystem::exists(outputPath) ||
            !std::filesystem::is_directory(outputPath)
            )
        {
            std::filesystem::create_directories(outputPath);
        }

        auto self = std::make_unique<Compilation>();

        self->Package              = std::move(package);
        self->OutputPath           = std::move(options.OutputPath);
        self->Natives              = std::make_unique<Ace::Natives>(self.get());
        self->GlobalScope          = std::make_shared<Scope>(self.get());
        self->TemplateInstantiator = std::make_unique<Ace::TemplateInstantiator>();
        self->LLVMContext          = std::make_unique<llvm::LLVMContext>();

        return Expected<std::unique_ptr<const Compilation>>
        {
            std::move(self)
        };
    }
}
