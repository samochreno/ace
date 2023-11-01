#include "Syntaxes/Exprs/StructConstructionExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Semas/Exprs/StructConstructionExprSema.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Syntaxes/Exprs/SymbolLiteralExprSyntax.hpp"
#include "Name.hpp"

namespace Ace
{
    StructConstructionExprSyntax::StructConstructionExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& typeName,
        std::vector<StructConstructionExprArg>&& args
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeName{ typeName },
        m_Args{ args }
    {
    }

    auto StructConstructionExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StructConstructionExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StructConstructionExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        SyntaxChildCollector collector{};

        std::for_each(begin(m_Args), end(m_Args),
        [&](const StructConstructionExprArg& arg)
        {
            collector.Collect(arg.OptValue);
        });

        return collector.Build();
    }

    static auto CreateArgValueSema(
        const std::shared_ptr<Scope>& scope,
        const StructConstructionExprArg& arg
    ) -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        if (arg.OptValue.has_value())
        {
            return arg.OptValue.value()->CreateExprSema();
        }

        const SymbolName symbolName
        {
            SymbolNameSection{ arg.Name },
            SymbolNameResolutionScope::Local,
        };

        return std::make_shared<const SymbolLiteralExprSyntax>(
            arg.Name.SrcLocation,
            scope,
            symbolName
        )->CreateSema();
    }

    static auto CreateFieldSymbolToUseSrcLocationsMap(
        const std::vector<FieldVarSymbol*>& fieldSymbols
    ) -> std::map<FieldVarSymbol*, std::vector<SrcLocation>>
    {
        std::map<FieldVarSymbol*, std::vector<SrcLocation>> map{};

        std::for_each(begin(fieldSymbols), end(fieldSymbols),
        [&](FieldVarSymbol* const fieldSymbol)
        {
            map[fieldSymbol] = std::vector<SrcLocation>{};
        });

        return map;
    }

    static auto DiagnoseInaccessibleVar(
        const std::shared_ptr<Scope>& scope,
        FieldVarSymbol* const fieldSymbol,
        const std::vector<SrcLocation>& fieldUseSrcLocations
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::for_each(begin(fieldUseSrcLocations), end(fieldUseSrcLocations),
        [&](const SrcLocation& useSrcLocation)
        {
            diagnostics.Collect(
                DiagnoseInaccessibleSymbol(useSrcLocation, fieldSymbol, scope)
            );
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseInaccessibleVars(
        const std::shared_ptr<Scope>& scope,
        const std::vector<FieldVarSymbol*>& fieldSymbols,
        const std::map<FieldVarSymbol*, std::vector<SrcLocation>>& fieldSymbolToUseSrcLocationsMap
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::for_each(begin(fieldSymbols), end(fieldSymbols),
        [&](FieldVarSymbol* const fieldSymbol)
        {
            diagnostics.Collect(DiagnoseInaccessibleVar(
                scope,
                fieldSymbol,
                fieldSymbolToUseSrcLocationsMap.at(fieldSymbol)
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseMissingVars(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        const std::vector<FieldVarSymbol*>& fieldSymbols,
        const std::map<FieldVarSymbol*, std::vector<SrcLocation>>& fieldSymbolToUseSrcLocationsMap
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<FieldVarSymbol*> missingFieldSymbols{};
        std::for_each(begin(fieldSymbols), end(fieldSymbols),
        [&](FieldVarSymbol* const fieldSymbol)
        {
            if (fieldSymbolToUseSrcLocationsMap.at(fieldSymbol).empty())
            {
                missingFieldSymbols.push_back(fieldSymbol);
            }
        });

        if (!missingFieldSymbols.empty())
        {
            diagnostics.Add(CreateMissingStructFieldsError(
                srcLocation,
                structSymbol,
                missingFieldSymbols
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseStructConstructionFieldSpecifiedMoreThanOnce(
        StructTypeSymbol* const structSymbol,
        FieldVarSymbol* const fieldSymbol,
        const std::vector<SrcLocation>& fieldUseSrcLocations
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (fieldUseSrcLocations.size() > 1)
        {
            std::for_each(
                begin(fieldUseSrcLocations) + 1,
                end  (fieldUseSrcLocations),
                [&](const SrcLocation& fieldUseSrcLocation)
                {
                    diagnostics.Add(CreateStructFieldInitializedMoreThanOnceError(
                        fieldUseSrcLocation,
                        fieldUseSrcLocations.front()
                    ));
                }
            );
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseFieldsSpecifiedMoreThanOnce(
        StructTypeSymbol* const structSymbol,
        const std::vector<FieldVarSymbol*>& fieldSymbols,
        const std::map<FieldVarSymbol*, std::vector<SrcLocation>>& fieldSymbolToUseSrcLocationsMap
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::for_each(begin(fieldSymbols), end(fieldSymbols),
        [&](FieldVarSymbol* const fieldSymbol)
        {
            diagnostics.Collect(DiagnoseStructConstructionFieldSpecifiedMoreThanOnce(
                structSymbol,
                fieldSymbol,
                fieldSymbolToUseSrcLocationsMap.at(fieldSymbol)
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto CreateArgs(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        StructTypeSymbol* const structSymbol,
        const std::vector<StructConstructionExprArg>& args
    ) -> Diagnosed<std::vector<StructConstructionExprSemaArg>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto fieldSymbols = structSymbol->CollectFields();

        auto fieldSymbolToUseSrcLocationsMap =
            CreateFieldSymbolToUseSrcLocationsMap(fieldSymbols);

        std::vector<StructConstructionExprSemaArg> semaArgs{};
        std::transform(begin(args), end(args), back_inserter(semaArgs),
        [&](const StructConstructionExprArg& arg) -> StructConstructionExprSemaArg
        {
            const auto matchingFieldSymbolIt = std::find_if(
                begin(fieldSymbols),
                end  (fieldSymbols),
                [&](FieldVarSymbol* const fieldSymbol)
                {
                    return fieldSymbol->GetName().String == arg.Name.String;
                }
            );

            const bool hasMatchingFieldSymbol =
                matchingFieldSymbolIt != end(fieldSymbols);

            auto* compilation = scope->GetCompilation();

            auto* const fieldSymbol = hasMatchingFieldSymbol ?
                *matchingFieldSymbolIt :
                compilation->GetErrorSymbols().GetField();

            if (hasMatchingFieldSymbol)
            {
                fieldSymbolToUseSrcLocationsMap.at(fieldSymbol).push_back(
                    arg.Name.SrcLocation
                );
            }
            else
            {
                diagnostics.Add(CreateStructHasNoFieldNamedError(
                    structSymbol,
                    arg.Name
                ));
            }

            const auto valueSema = diagnostics.Collect(
                CreateArgValueSema(scope, arg)
            );

            return StructConstructionExprSemaArg{ fieldSymbol, valueSema };
        });

        diagnostics.Collect(DiagnoseInaccessibleVars(
            scope,
            fieldSymbols,
            fieldSymbolToUseSrcLocationsMap
        ));
        diagnostics.Collect(DiagnoseMissingVars(
            srcLocation,
            structSymbol,
            fieldSymbols,
            fieldSymbolToUseSrcLocationsMap
        ));
        diagnostics.Collect(DiagnoseFieldsSpecifiedMoreThanOnce(
            structSymbol,
            fieldSymbols,
            fieldSymbolToUseSrcLocationsMap
        ));

        return Diagnosed{ semaArgs, std::move(diagnostics) };
    }

    auto StructConstructionExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const StructConstructionExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optStructSymbol = diagnostics.Collect(
            GetScope()->ResolveStaticSymbol<StructTypeSymbol>(m_TypeName)
        );
        auto* const structSymbol = optStructSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetStruct()
        );

        const auto semaArgs = [&]() -> std::vector<StructConstructionExprSemaArg>
        {
            if (!optStructSymbol.has_value())
            {
                return {};
            }

            return diagnostics.Collect(CreateArgs(
                m_TypeName.CreateSrcLocation(),
                GetScope(),
                structSymbol,
                m_Args
            ));
        }();

        return Diagnosed
        {
            std::make_shared<const StructConstructionExprSema>(
                GetSrcLocation(),
                GetScope(),
                structSymbol,
                semaArgs
            ),
            std::move(diagnostics),
        };
    }

    auto StructConstructionExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
