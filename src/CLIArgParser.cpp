#include "CLIArgParser.hpp"

#include <vector>
#include <string_view>
#include <optional>
#include <map>

#include "Diagnostic.hpp"
#include "Diagnostics/CLIArgDiagnostics.hpp"
#include "CLIArgBuffer.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    using CLIArgParseIterator = std::vector<std::string_view>::const_iterator;

    template<typename T>
    class CLIArgParseResult
    {
    public:
        CLIArgParseResult(
            const T& t_value,
            const CLIArgParseIterator t_endIt
        ) : Value{ t_value },
            EndIterator{ t_endIt }
        {
        }

        T Value{};
        CLIArgParseIterator EndIterator{};
    };

    class CLIArgParser
    {
    public:
        CLIArgParser(
            const CLIArgBuffer* const t_argBuffer,
            std::vector<const CLIOptionDefinition*>&& t_optionDefinitions
        ) : m_ArgBuffer{ t_argBuffer },
            m_OptionDefinitions{ std::move(t_optionDefinitions) },
            m_BeginIterator{ begin(t_argBuffer->GetArgs()) },
            m_Iterator{ begin(t_argBuffer->GetArgs()) },
            m_EndIterator{ end(t_argBuffer->GetArgs()) }
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

        auto Peek(const size_t t_distance = 1) const -> std::string_view
        {
            return *((m_Iterator - 1) + t_distance);
        }

        auto Eat(const size_t t_count = 1) -> void
        {
            m_Iterator += t_count;
        }
        template<typename T>
        auto Eat(const CLIArgParseResult<T>& t_result) -> void
        {
            m_Iterator = t_result.EndIterator;
        }

        template<typename T>
        auto Build(const T& t_value) -> CLIArgParseResult<T>
        {
            return CLIArgParseResult<T>{ t_value, m_Iterator };
        }

    private:
        const CLIArgBuffer* m_ArgBuffer{};
        std::vector<const CLIOptionDefinition*> m_OptionDefinitions{};
        CLIArgParseIterator m_BeginIterator{};
        CLIArgParseIterator m_Iterator{};
        CLIArgParseIterator m_EndIterator{};
    };

    static const CLIOptionDefinition ErrorOptionDefinition
    {
        std::nullopt,
        "",
        CLIOptionKind::WithValue,
    };

    static auto DiagnoseMissingOptionName(
        const CLIArgParser& t_parser,
        const std::string_view t_name
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

        if (!t_name.empty())
        {
            return diagnosticBag;
        }

        const SourceLocation sourceLocation
        {
            t_parser.GetArgBuffer(),
            begin(t_parser.Peek()),
            end  (t_parser.Peek()),
        };

        return diagnosticBag.Add(CreateMissingCLIOptionNameError(
            sourceLocation
        ));
    }

    static auto DiagnoseMissingOrUnexpectedOptionValue(
        const CLIArgParser& t_parser,
        const CLIOptionDefinition* const t_definition,
        const std::optional<std::string_view>& t_optValue
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

        if (
            (t_definition->Kind == CLIOptionKind::WithValue) &&
            !t_optValue.has_value()
            )
        {
            SourceLocation sourceLocation
            {
                t_parser.GetArgBuffer(),
                begin(t_parser.Peek()),
                end  (t_parser.Peek()),
            };

            return diagnosticBag.Add(CreateMissingCLIOptionValueError(
                sourceLocation
            ));
        }

        if (
            (t_definition->Kind == CLIOptionKind::WithoutValue) &&
            t_optValue.has_value()
            )
        {
            SourceLocation sourceLocation
            {
                t_parser.GetArgBuffer(),
                begin(t_optValue.value()),
                end  (t_optValue.value()),
            };

            return diagnosticBag.Add(CreateUnexpectedCLIOptionValueError(
                sourceLocation
            ));
        }

        return diagnosticBag;
    }

    static auto ParseLongOption(
        CLIArgParser t_parser
    ) -> Diagnosed<CLIArgParseResult<CLIOption>>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(t_parser.Peek().at(0) == '-');
        ACE_ASSERT(t_parser.Peek().at(1) == '-');

        const std::string_view name
        {
            begin(t_parser.Peek()) + 2,
            std::find(begin(t_parser.Peek()), end(t_parser.Peek()), '='),
        };

        diagnosticBag.Add(DiagnoseMissingOptionName(
            t_parser,
            name
        ));

        const auto matchedDefinitionIt = std::find_if(
            begin(t_parser.GetOptionDefinitions()),
            end  (t_parser.GetOptionDefinitions()),
            [&](const CLIOptionDefinition* const t_optionDefinition)
            {
                return t_optionDefinition->LongName == name;
            }
        );
        if (matchedDefinitionIt == end(t_parser.GetOptionDefinitions()))
        {
            SourceLocation sourceLocation
            {
                t_parser.GetArgBuffer(),
                begin(name),
                end  (name),
            };

            diagnosticBag.Add(CreateUnknownCLIOptionNameError(
                sourceLocation
            ));
        }

        const auto* const definition =
            (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error) ?
            &ErrorOptionDefinition : 
            *matchedDefinitionIt;

        const bool isValueInFirstArg = end(name) != end(t_parser.Peek());

        const auto optValue = [&]() -> std::optional<std::string_view>
        {
            if (isValueInFirstArg)
            {
                const std::string_view value
                {
                    end(name) + 1,
                    end(t_parser.Peek()),
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

            return t_parser.Peek(2);
        }();

        diagnosticBag.Add(DiagnoseMissingOrUnexpectedOptionValue(
            t_parser,
            definition,
            optValue
        ));

        const CLIOption option
        {
            definition,
            optValue,
        };

        const size_t length =
            (!optValue.has_value() || isValueInFirstArg) ? 1 : 2;

        t_parser.Eat(length);

        return Diagnosed{ t_parser.Build(option), diagnosticBag };
    }

    static auto ParseShortOption(
        CLIArgParser t_parser
    ) -> Diagnosed<CLIArgParseResult<CLIOption>>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(t_parser.Peek().at(0) == '-');
        ACE_ASSERT(t_parser.Peek().at(1) != '-');

        const std::string_view name
        {
            begin(t_parser.Peek()) + 1,
            begin(t_parser.Peek()) + 2,
        };

        diagnosticBag.Add(DiagnoseMissingOptionName(
            t_parser,
            name
        ));

        const auto matchedDefinitionIt = std::find_if(
            begin(t_parser.GetOptionDefinitions()),
            end  (t_parser.GetOptionDefinitions()),
            [&](const CLIOptionDefinition* const t_optionDefinition)
            {
                return t_optionDefinition->ShortName == name;
            }
        );
        if (matchedDefinitionIt == end(t_parser.GetOptionDefinitions()))
        {
            SourceLocation sourceLocation
            {
                t_parser.GetArgBuffer(),
                begin(name),
                end  (name),
            };

            diagnosticBag.Add(CreateUnknownCLIOptionNameError(
                sourceLocation
            ));
        }

        const auto* const definition =
            (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error) ?
            &ErrorOptionDefinition : 
            *matchedDefinitionIt;

        const bool isValueInFirstArg = t_parser.Peek().size() > 2;

        const auto optValue = [&]() -> std::optional<std::string_view>
        {
            if (isValueInFirstArg)
            {
                return std::string_view
                {
                    end(name),
                    end(t_parser.Peek()),
                };
            }

            if (definition->Kind == CLIOptionKind::WithoutValue)
            {
                return std::nullopt;
            }

            return t_parser.Peek(2);
        }();

        diagnosticBag.Add(DiagnoseMissingOrUnexpectedOptionValue(
            t_parser,
            definition,
            optValue
        ));

        const CLIOption option
        {
            definition,
            optValue,
        };

        const size_t length =
            (!optValue.has_value() || isValueInFirstArg) ? 1 : 2;

        t_parser.Eat(length);

        return Diagnosed{ t_parser.Build(option), diagnosticBag };
    }

    static auto ParseOption(
        const CLIArgParser& t_parser
    ) -> Diagnosed<CLIArgParseResult<CLIOption>>
    {
        ACE_ASSERT(t_parser.Peek().at(0) == '-');

        return (t_parser.Peek().at(1) != '-') ? 
            ParseShortOption(t_parser) :
            ParseLongOption(t_parser);
    }

    static auto CreateDefaultOptionMap(
        const CLIArgParser& t_parser
    ) -> std::map<const CLIOptionDefinition*, CLIOption>
    {
        std::map<const CLIOptionDefinition*, CLIOption> optionMap{};

        std::for_each(
            begin(t_parser.GetOptionDefinitions()),
            end  (t_parser.GetOptionDefinitions()),
            [&](const CLIOptionDefinition* const t_optionDefinition)
            {
                if (t_optionDefinition->OptDefaultValue.has_value())
                {
                    return;
                }

                optionMap[t_optionDefinition] = CLIOption
                {
                    t_optionDefinition,
                    t_optionDefinition->OptDefaultValue,
                };
            }
        );

        return optionMap;
    }

    auto ParseCommandLineArgs(
        const CLIArgBuffer* const t_argBuffer,
        std::vector<const CLIOptionDefinition*>&& t_optionDefinitions
    ) -> Expected<CLIArgsParseResult>
    {
        DiagnosticBag diagnosticBag{};

        CLIArgParser parser
        {
            t_argBuffer,
            std::move(t_optionDefinitions),
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

            optionMap[dgnOption.Unwrap().Value.Definition] =
                dgnOption.Unwrap().Value;

            parser.Eat(dgnOption.Unwrap());
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
