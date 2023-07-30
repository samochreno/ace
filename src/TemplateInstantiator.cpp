#include "TemplateInstantiator.hpp"

#include "Symbols/Templates/TemplateSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Symbols/TemplatableSymbol.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    TemplateInstantiator::TemplateInstantiator()
        : m_Symbols{}, m_SymbolOnlyInstantiatedASTsMap{}
    {
    }

    TemplateInstantiator::~TemplateInstantiator()
    {
    }

    auto TemplateInstantiator::SetSymbols(
        const std::vector<ITemplateSymbol*>& symbols
    ) -> void
    {
        m_Symbols = symbols;
    }

    auto TemplateInstantiator::InstantiatePlaceholderSymbols() -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        std::for_each(begin(m_Symbols), end(m_Symbols),
        [&](ITemplateSymbol* const symbol)
        {
            const auto implParams = symbol->CollectImplParams();
            std::vector<ITypeSymbol*> upcastedImplParams
            {
                begin(implParams),
                end  (implParams),
            };

            const auto params = symbol->CollectParams();
            std::vector<ITypeSymbol*> upcastedParams
            {
                begin(params),
                end  (params),
            };

            const auto dgnPlaceholderSymbol = InstantiateSymbols(
                symbol,
                upcastedImplParams,
                upcastedParams
            );
            diagnostics.Add(dgnPlaceholderSymbol);
            symbol->SetPlaceholderSymbol(dgnPlaceholderSymbol.Unwrap());
        });

        return Diagnosed<void>{ diagnostics };
    }

    static auto IsArgPlaceholder(
        ITypeSymbol* const arg
    ) -> bool;

    static auto AreArgsPlaceholders(
        const std::vector<ITypeSymbol*>& implArgs,
        const std::vector<ITypeSymbol*>& args
    ) -> bool
    {
        const auto placeholderImplArgIt = std::find_if(
            begin(implArgs),
            end  (implArgs),
            [](ITypeSymbol* const implArg)
            {
                return IsArgPlaceholder(implArg);
            }
        );
        if (placeholderImplArgIt != end(implArgs))
        {
            return true;
        }

        const auto placeholderArgIt = std::find_if(
            begin(args),
            end  (args),
            [](ITypeSymbol* const arg)
            {
                return IsArgPlaceholder(arg);
            }
        );
        if (placeholderArgIt != end(args))
        {
            return true;
        }

        return false;
    }

    static auto IsArgPlaceholder(
        ITypeSymbol* arg
    ) -> bool
    {
        arg = arg->GetUnaliased();

        if (dynamic_cast<ImplTemplateParamTypeSymbol*>(arg))
            return true;

        if (dynamic_cast<NormalTemplateParamTypeSymbol*>(arg))
            return true;

        auto* const templatable =
            dynamic_cast<ITemplatableSymbol*>(arg);
        if (!templatable)
        {
            return false;
        }

        return AreArgsPlaceholders(
            templatable->CollectImplTemplateArgs(),
            templatable->CollectTemplateArgs()
        );
    }

    auto TemplateInstantiator::InstantiateSymbols(
        ITemplateSymbol* const t3mplate,
        const std::vector<ITypeSymbol*>& implArgs,
        const std::vector<ITypeSymbol*>& args
    ) -> Diagnosed<ISymbol*>
    {
        DiagnosticBag diagnostics{};

        const auto dgnSymbolsInstantiationResult = t3mplate->InstantiateSymbols(
            implArgs,
            args
        );
        diagnostics.Add(dgnSymbolsInstantiationResult);

        const bool areArgsPlaceholders = AreArgsPlaceholders(
            implArgs,
            args
        );
        if (!areArgsPlaceholders)
        {
            m_SymbolOnlyInstantiatedASTsMap[t3mplate].push_back(
                dgnSymbolsInstantiationResult.Unwrap().AST
            );
        }
        
        return Diagnosed
        {
            dgnSymbolsInstantiationResult.Unwrap().Symbol,
            diagnostics,
        };
    }

    auto TemplateInstantiator::InstantiateSemanticsForSymbols() -> void
    {
        bool didInstantiateSemantics = true;
        while (didInstantiateSemantics)
        {
            auto symbolOnlyInstantiatedASTsMap = m_SymbolOnlyInstantiatedASTsMap;
            m_SymbolOnlyInstantiatedASTsMap.clear();

            didInstantiateSemantics = false;

            for (const auto& templateASTsPair : symbolOnlyInstantiatedASTsMap)
            {
                didInstantiateSemantics = true;
                
                std::for_each(
                    begin(templateASTsPair.second),
                    end  (templateASTsPair.second),
                    [&](const std::shared_ptr<const INode>& ast)
                    {
                        templateASTsPair.first->InstantiateSemanticsForSymbols(ast);
                    }
                );
            }
        }
    }
}
