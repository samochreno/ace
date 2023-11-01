#include "CLIArgParser.hpp"

#include <vector>
#include <string_view>
#include <optional>
#include <map>

#include "Diagnostic.hpp"
#include "Diagnostics/CLIArgDiagnostics.hpp"
#include "CLIArgBuffer.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    using CLIArgParseIterator = std::vector<std::string_view>::const_iterator;

    class CLIArgParser
    {
    public:
        CLIArgParser(
            const CLIArgBuffer* const argBuffer,
            std::vector<const CLIOptionDefinition*>&& optionDefinitions
        ) : m_ArgBuffer{ argBuffer },
            m_OptionDefinitions{ std::move(optionDefinitions) },
            m_Iterator{ begin(argBuffer->GetArgs()) },
            m_EndIterator{ end(argBuffer->GetArgs()) }
        {
        }
        ~CLIArgParser() = default;

        auto GetArgBuffer() const -> const CLIArgBuffer*
        {
            return m_ArgBuffer;
        }
        auto GetOptionDefinitions() const -> const std::vector<const CLIOptionDefinition*>&
        {
            return m_OptionDefinitions;
        }
        auto IsEnd(const size_t distance = 0) const -> bool
        {
            return (m_Iterator + distance) == m_EndIterator;
        }
        auto Peek(const size_t distance = 0) const -> std::string_view
        {
            return *(m_Iterator + distance);
        }

        auto Eat() -> void
        {
            m_Iterator++;
            ACE_ASSERT(m_Iterator <= m_EndIterator);
        }

    private:
        const CLIArgBuffer* m_ArgBuffer{};
        std::vector<const CLIOptionDefinition*> m_OptionDefinitions{};
        std::vector<std::string_view>::const_iterator m_Iterator{};
        std::vector<std::string_view>::const_iterator m_EndIterator{};
    };

    static const CLIOptionDefinition ErrorOptionDefinition
    {
        std::nullopt,
        "",
        CLIOptionKind::WithValue,
    };

    static auto DiagnoseMissingOptionName(
        const CLIArgParser& parser,
        const std::string_view name
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!name.empty())
        {
            return std::move(diagnostics);
        }

        const SrcLocation srcLocation
        {
            parser.GetArgBuffer(),
            begin(parser.Peek()),
            end  (parser.Peek()),
        };

        diagnostics.Add(CreateMissingCLIOptionNameError(srcLocation));
        return std::move(diagnostics);
    }

    static auto DiagnoseMissingOrUnexpectedOptionValue(
        const CLIArgParser& parser,
        const CLIOptionDefinition* const definition,
        const std::optional<std::string_view>& optValue
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (
            (definition->Kind == CLIOptionKind::WithValue) &&
            !optValue.has_value()
            )
        {
            SrcLocation srcLocation
            {
                parser.GetArgBuffer(),
                begin(parser.Peek()),
                end  (parser.Peek()),
            };

            diagnostics.Add(CreateMissingCLIOptionValueError(srcLocation));
        }

        if (
            (definition->Kind == CLIOptionKind::WithoutValue) &&
            optValue.has_value()
            )
        {
            SrcLocation srcLocation
            {
                parser.GetArgBuffer(),
                begin(optValue.value()),
                end  (optValue.value()),
            };

            diagnostics.Add(CreateUnexpectedCLIOptionValueError(srcLocation));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto CreateLongOptionValue(
        const CLIArgParser& parser,
        const std::string_view name,
        const bool isValueInFirstArg,
        const CLIOptionDefinition* const definition
    ) -> std::optional<std::string_view>
    {
        if (isValueInFirstArg)
        {
            const std::string_view value
            {
                end(name) + 1,
                end(parser.Peek()),
            };

            if (value.empty())
            {
                return std::nullopt;
            }

            return value;
        }

        if (parser.IsEnd(1))
        {
            return std::nullopt;
        }

        if (definition->Kind == CLIOptionKind::WithoutValue)
        {
            return std::nullopt;
        }

        return parser.Peek(1);
    }

    static auto ParseLongOption(CLIArgParser& parser) -> Diagnosed<CLIOption>
    {
        auto diagnostics = DiagnosticBag::Create();

        ACE_ASSERT(parser.Peek().at(0) == '-');
        ACE_ASSERT(parser.Peek().at(1) == '-');

        const std::string_view name
        {
            begin(parser.Peek()) + 2,
            std::find(begin(parser.Peek()), end(parser.Peek()), '='),
        };

        diagnostics.Collect(DiagnoseMissingOptionName(parser, name));

        const auto matchedDefinitionIt = std::find_if(
            begin(parser.GetOptionDefinitions()),
            end  (parser.GetOptionDefinitions()),
            [&](const CLIOptionDefinition* const optionDefinition)
            {
                return optionDefinition->LongName == name;
            }
        );
        if (matchedDefinitionIt == end(parser.GetOptionDefinitions()))
        {
            SrcLocation srcLocation
            {
                parser.GetArgBuffer(),
                begin(name),
                end  (name),
            };

            diagnostics.Add(CreateUnknownCLIOptionNameError(srcLocation));
        }

        const auto* const definition = diagnostics.HasErrors() ?
            &ErrorOptionDefinition : 
            *matchedDefinitionIt;

        const bool isValueInFirstArg = end(name) != end(parser.Peek());

        const auto optValue =
            CreateLongOptionValue(parser, name, isValueInFirstArg, definition);

        diagnostics.Collect(
            DiagnoseMissingOrUnexpectedOptionValue(parser, definition, optValue)
        );

        const CLIOption option { definition, optValue };

        parser.Eat();
        if (optValue.has_value() && !isValueInFirstArg)
        {
            parser.Eat();
        }

        return Diagnosed{ option, std::move(diagnostics) };
    }

    static auto CreateShortOptionValue(
        const CLIArgParser& parser,
        const std::string_view name,
        const bool isValueInFirstArg,
        const CLIOptionDefinition* const definition
    ) -> std::optional<std::string_view>
    {
        if (isValueInFirstArg)
        {
            return std::string_view
            {
                end(name),
                end(parser.Peek()),
            };
        }

        if (parser.IsEnd(1))
        {
            return std::nullopt;
        }

        if (definition->Kind == CLIOptionKind::WithoutValue)
        {
            return std::nullopt;
        }

        return parser.Peek(1);
    }

    static auto ParseShortOption(CLIArgParser& parser) -> Diagnosed<CLIOption>
    {
        auto diagnostics = DiagnosticBag::Create();

        ACE_ASSERT(parser.Peek().at(0) == '-');
        ACE_ASSERT(parser.Peek().at(1) != '-');

        const std::string_view name
        {
            begin(parser.Peek()) + 1,
            begin(parser.Peek()) + 2,
        };

        diagnostics.Collect(DiagnoseMissingOptionName(parser, name));

        const auto matchedDefinitionIt = std::find_if(
            begin(parser.GetOptionDefinitions()),
            end  (parser.GetOptionDefinitions()),
            [&](const CLIOptionDefinition* const optionDefinition)
            {
                return optionDefinition->ShortName == name;
            }
        );
        if (matchedDefinitionIt == end(parser.GetOptionDefinitions()))
        {
            SrcLocation srcLocation
            {
                parser.GetArgBuffer(),
                begin(name),
                end  (name),
            };

            diagnostics.Add(CreateUnknownCLIOptionNameError(srcLocation));
        }

        const auto* const definition = diagnostics.HasErrors() ?
            &ErrorOptionDefinition : 
            *matchedDefinitionIt;

        const bool isValueInFirstArg = parser.Peek().size() > 2;

        const auto optValue =
            CreateShortOptionValue(parser, name, isValueInFirstArg, definition);

        diagnostics.Collect(
            DiagnoseMissingOrUnexpectedOptionValue(parser, definition, optValue)
        );

        const CLIOption option{ definition, optValue };

        parser.Eat();
        if (optValue.has_value() && !isValueInFirstArg)
        {
            parser.Eat();
        }

        return Diagnosed{ option, std::move(diagnostics) };
    }

    static auto ParseOption(CLIArgParser& parser) -> Diagnosed<CLIOption>
    {
        ACE_ASSERT(parser.Peek().at(0) == '-');

        return (parser.Peek().at(1) != '-') ? 
            ParseShortOption(parser) :
            ParseLongOption(parser);
    }

    static auto CreateDefaultOptionMap(
        const CLIArgParser& parser
    ) -> std::map<const CLIOptionDefinition*, CLIOption>
    {
        std::map<const CLIOptionDefinition*, CLIOption> optionMap{};

        std::for_each(
            begin(parser.GetOptionDefinitions()),
            end  (parser.GetOptionDefinitions()),
            [&](const CLIOptionDefinition* const optionDefinition)
            {
                if (optionDefinition->OptDefaultValue.has_value())
                {
                    return;
                }

                optionMap[optionDefinition] = CLIOption
                {
                    optionDefinition,
                    optionDefinition->OptDefaultValue,
                };
            }
        );

        return optionMap;
    }

    auto ParseCommandLineArgs(
        const CLIArgBuffer* const argBuffer,
        std::vector<const CLIOptionDefinition*>&& optionDefinitions
    ) -> Expected<CLIArgsParseResult>
    {
        auto diagnostics = DiagnosticBag::Create();

        CLIArgParser parser{ argBuffer, std::move(optionDefinitions) };

        auto optionMap = CreateDefaultOptionMap(parser);
        std::vector<std::string_view> positionalArgs{};
        while (!parser.IsEnd())
        {
            if (!parser.Peek().starts_with('-'))
            {
                positionalArgs.push_back(parser.Peek());
                parser.Eat();
                continue;
            }

            const auto option = diagnostics.Collect(ParseOption(parser));
            optionMap[option.Definition] = option;
        }

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            CLIArgsParseResult
            {
                std::move(positionalArgs),
                std::move(optionMap),
            },
            std::move(diagnostics),
        };
    }
}
