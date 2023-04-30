#pragma once

#include <vector>
#include <string>

#include "Token.hpp"
#include "Error.hpp"
#include "ParseData.hpp"
#include "Compilation.hpp"

namespace Ace::Scanning
{
    enum class Kind
    {
        None,
        Language,
        Metadata,
    };

    struct Context
    {
        Context(
            const Compilation& t_compilation,
            const Scanning::Kind& t_scanningKind,
            const std::string::const_iterator t_iterator,
            const std::string::const_iterator t_iteratorEnd
        ) : Compilation{ t_compilation },
            ScanningKind{ t_scanningKind },
            Iterator{ t_iterator },
            IteratorEnd{ t_iteratorEnd }
        {
        }

        const Compilation& Compilation;
        Scanning::Kind ScanningKind{};
        const std::string::const_iterator Iterator{};
        const std::string::const_iterator IteratorEnd{};
    };

    class Scanner
    {
    public:
        Scanner() = delete;

        static auto ScanTokens(
            const Compilation& t_compilation,
            const Scanning::Kind& t_scanningKind,
            const std::string& t_string
        ) -> Expected<std::vector<Token>>;

    private:
        static auto ScanToken(
            Context t_context
        ) -> Expected<ParseData<std::vector<Token>>>;
        static auto ScanDefault(
            Context t_context
        ) -> Expected<ParseData<std::vector<Token>>>;
        static auto ScanIdentifier(
            Context t_context
        ) -> Expected<ParseData<std::vector<Token>>>;
        static auto ScanNumber(
            Context t_context
        ) -> Expected<ParseData<std::vector<Token>>>;
        static auto ScanComment(
            Context t_context
        ) -> Expected<ParseData<std::vector<Token>>>;
        static auto ScanMultilineComment(
            Context t_context
        ) -> Expected<ParseData<std::vector<Token>>>;
    };
}
