#include "TemplateInstantiator.hpp"

#include "Symbols/Templates/TemplateSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Symbols/TemplatableSymbol.hpp"
#include "Diagnostics.hpp"

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
        const std::vector<ITemplateSymbol*>& t_symbols
    ) -> void
    {
        m_Symbols = t_symbols;
    }

    auto TemplateInstantiator::InstantiatePlaceholderSymbols() -> Expected<void>
    {
        return TransformExpectedVector(m_Symbols,
        [&](ITemplateSymbol* const t_symbol) -> Expected<void>
        {
            const auto implParams = t_symbol->CollectImplParams();
            std::vector<ITypeSymbol*> upcastedImplParams
            {
                begin(implParams),
                end  (implParams),
            };

            const auto params = t_symbol->CollectParams();
            std::vector<ITypeSymbol*> upcastedParams
            {
                begin(params),
                end  (params),
            };

            ACE_TRY(placeholderSymbol, InstantiateSymbols(
                t_symbol,
                upcastedImplParams,
                upcastedParams
            ));
            t_symbol->SetPlaceholderSymbol(placeholderSymbol);

            return Void{};
        });
    }

    static auto IsArgPlaceholder(
        ITypeSymbol* const t_arg
    ) -> bool;

    static auto AreArgsPlaceholders(
        const std::vector<ITypeSymbol*>& t_implArgs,
        const std::vector<ITypeSymbol*>& t_args
    ) -> bool
    {
        const auto placeholderImplArgIt = std::find_if(
            begin(t_implArgs),
            end  (t_implArgs),
            [](ITypeSymbol* const t_implArg)
            {
                return IsArgPlaceholder(t_implArg);
            }
        );
        if (placeholderImplArgIt != end(t_implArgs))
        {
            return true;
        }

        const auto placeholderArgIt = std::find_if(
            begin(t_args),
            end  (t_args),
            [](ITypeSymbol* const t_arg)
            {
                return IsArgPlaceholder(t_arg);
            }
        );
        if (placeholderArgIt != end(t_args))
        {
            return true;
        }

        return false;
    }

    static auto IsArgPlaceholder(
        ITypeSymbol* t_arg
    ) -> bool
    {
        t_arg = t_arg->GetUnaliased();

        if (dynamic_cast<ImplTemplateParamTypeSymbol*>(t_arg))
            return true;

        if (dynamic_cast<NormalTemplateParamTypeSymbol*>(t_arg))
            return true;

        auto* const templatable =
            dynamic_cast<ITemplatableSymbol*>(t_arg);
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
        ITemplateSymbol* const t_template,
        const std::vector<ITypeSymbol*>& t_implArgs,
        const std::vector<ITypeSymbol*>& t_args
    ) -> Expected<ISymbol*>
    {
        ACE_TRY(symbolsInstantiationResult, t_template->InstantiateSymbols(
            t_implArgs,
            t_args
        ));

        const bool areArgsPlaceholders = AreArgsPlaceholders(
            t_implArgs,
            t_args
        );
        if (!areArgsPlaceholders)
        {
            m_SymbolOnlyInstantiatedASTsMap[t_template].push_back(
                symbolsInstantiationResult.AST
            );
        }
        
        return symbolsInstantiationResult.Symbol;
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
                    [&](const std::shared_ptr<const INode>& t_ast)
                    {
                        templateASTsPair.first->InstantiateSemanticsForSymbols(t_ast);
                    }
                );
            }
        }
    }
}
