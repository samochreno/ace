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
        auto IsEnd() const -> bool
        {
            return m_Iterator == m_EndIterator;
        }
        auto Peek(const size_t distance = 1) const -> std::string_view
        {
            return *((m_Iterator - 1) + distance);
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
        DiagnosticBag diagnosticBag{};

        if (!name.empty())
        {
            return diagnosticBag;
        }

        const SrcLocation srcLocation
        {
            parser.GetArgBuffer(),
            begin(parser.Peek()),
            end  (parser.Peek()),
        };

        return diagnosticBag.Add(CreateMissingCLIOptionNameError(
            srcLocation
        ));
    }

    static auto DiagnoseMissingOrUnexpectedOptionValue(
        const CLIArgParser& parser,
        const CLIOptionDefinition* const definition,
        const std::optional<std::string_view>& optValue
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

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

            return diagnosticBag.Add(CreateMissingCLIOptionValueError(
                srcLocation
            ));
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

            return diagnosticBag.Add(CreateUnexpectedCLIOptionValueError(
                srcLocation
            ));
        }

        return diagnosticBag;
    }

    static auto ParseLongOption(
        CLIArgParser& parser
    ) -> Diagnosed<CLIOption>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(parser.Peek().at(0) == '-');
        ACE_ASSERT(parser.Peek().at(1) == '-');

        const std::string_view name
        {
            begin(parser.Peek()) + 2,
            std::find(begin(parser.Peek()), end(parser.Peek()), '='),
        };

        diagnosticBag.Add(DiagnoseMissingOptionName(
            parser,
            name
        ));

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

            diagnosticBag.Add(CreateUnknownCLIOptionNameError(
                srcLocation
            ));
        }

        const auto* const definition =
            (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error) ?
            &ErrorOptionDefinition : 
            *matchedDefinitionIt;

        const bool isValueInFirstArg = end(name) != end(parser.Peek());

        const auto optValue = [&]() -> std::optional<std::string_view>
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

            if (definition->Kind == CLIOptionKind::WithoutValue)
            {
                return std::nullopt;
            }

            return parser.Peek(1);
        }();

        diagnosticBag.Add(DiagnoseMissingOrUnexpectedOptionValue(
            parser,
            definition,
            optValue
        ));

        const CLIOption option
        {
            definition,
            optValue,
        };

        parser.Eat();
        if (optValue.has_value() && !isValueInFirstArg)
        {
            parser.Eat();
        }

        return Diagnosed{ option, diagnosticBag };
    }

    static auto ParseShortOption(
        CLIArgParser& parser
    ) -> Diagnosed<CLIOption>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(parser.Peek().at(0) == '-');
        ACE_ASSERT(parser.Peek().at(1) != '-');

        const std::string_view name
        {
            begin(parser.Peek()) + 1,
            begin(parser.Peek()) + 2,
        };

        diagnosticBag.Add(DiagnoseMissingOptionName(
            parser,
            name
        ));

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

            diagnosticBag.Add(CreateUnknownCLIOptionNameError(
                srcLocation
            ));
        }

        const auto* const definition =
            (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error) ?
            &ErrorOptionDefinition : 
            *matchedDefinitionIt;

        const bool isValueInFirstArg = parser.Peek().size() > 2;

        const auto optValue = [&]() -> std::optional<std::string_view>
        {
            if (isValueInFirstArg)
            {
                return std::string_view
                {
                    end(name),
                    end(parser.Peek()),
                };
            }

            if (definition->Kind == CLIOptionKind::WithoutValue)
            {
                return std::nullopt;
            }

            return parser.Peek(1);
        }();

        diagnosticBag.Add(DiagnoseMissingOrUnexpectedOptionValue(
            parser,
            definition,
            optValue
        ));

        const CLIOption option
        {
            definition,
            optValue,
        };

        parser.Eat();
        if (optValue.has_value() && !isValueInFirstArg)
        {
            parser.Eat();
        }

        return Diagnosed{ option, diagnosticBag };
    }

    static auto ParseOption(
        CLIArgParser& parser
    ) -> Diagnosed<CLIOption>
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
        DiagnosticBag diagnosticBag{};

        CLIArgParser parser
        {
            argBuffer,
            std::move(optionDefinitions),
        };

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

            const auto dgnOption = ParseOption(parser);
            diagnosticBag.Add(dgnOption);

            optionMap[dgnOption.Unwrap().Definition] = dgnOption.Unwrap();
        }

        return Expected
        {
            CLIArgsParseResult
            {
                std::move(positionalArgs),
                std::move(optionMap),
            },
            diagnosticBag,
        };
    }
}
