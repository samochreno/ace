#include "TemplateInstantiator.hpp"

#include "Symbol/Template/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/TemplateParameter/Impl.hpp"
#include "Symbol/Type/TemplateParameter/Normal.hpp"
#include "Symbol/Templatable.hpp"
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
        const std::vector<Symbol::Template::IBase*>& t_symbols
    ) -> void
    {
        m_Symbols = t_symbols;
    }

    auto TemplateInstantiator::InstantiatePlaceholderSymbols() -> Expected<void>
    {
        return TransformExpectedVector(m_Symbols,
        [&](Symbol::Template::IBase* const t_symbol) -> Expected<void>
        {
            const auto implParameters = t_symbol->CollectImplParameters();
            std::vector<Symbol::Type::IBase*> upcastedImplParameters
            {
                begin(implParameters),
                end  (implParameters),
            };

            const auto parameters = t_symbol->CollectParameters();
            std::vector<Symbol::Type::IBase*> upcastedParameters
            {
                begin(parameters),
                end  (parameters),
            };

            ACE_TRY(placeholderSymbol, InstantiateSymbols(
                t_symbol,
                upcastedImplParameters,
                upcastedParameters
            ));
            t_symbol->SetPlaceholderSymbol(placeholderSymbol);

            return Void;
        });
    }

    static auto IsArgumentPlaceholder(
        Symbol::Type::IBase* const t_argument
    ) -> bool;

    static auto AreArgumentsPlaceholders(
        const std::vector<Symbol::Type::IBase*>& t_implArguments,
        const std::vector<Symbol::Type::IBase*>& t_arguments
    ) -> bool
    {
        const auto foundImplIt = std::find_if(
            begin(t_implArguments),
            end  (t_implArguments),
            [](Symbol::Type::IBase* const t_implArgument)
            {
                return IsArgumentPlaceholder(t_implArgument);
            }
        );
        if (foundImplIt != end(t_implArguments))
            return true;

        const auto foundIt = std::find_if(
            begin(t_arguments),
            end  (t_arguments),
            [](Symbol::Type::IBase* const t_argument)
            {
                return IsArgumentPlaceholder(t_argument);
            }
        );
        if (foundIt != end(t_arguments))
            return true;

        return false;
    }

    static auto IsArgumentPlaceholder(
        Symbol::Type::IBase* t_argument
    ) -> bool
    {
        t_argument = t_argument->GetUnaliased();

        if (dynamic_cast<Symbol::Type::TemplateParameter::Impl*>(t_argument))
            return true;

        if (dynamic_cast<Symbol::Type::TemplateParameter::Normal*>(t_argument))
            return true;

        auto* const templatable =
            dynamic_cast<Symbol::ITemplatable*>(t_argument);
        if (!templatable)
        {
            return false;
        }

        return AreArgumentsPlaceholders(
            templatable->CollectImplTemplateArguments(),
            templatable->CollectTemplateArguments()
        );
    }

    auto TemplateInstantiator::InstantiateSymbols(
        Symbol::Template::IBase* const t_template,
        const std::vector<Symbol::Type::IBase*>& t_implArguments,
        const std::vector<Symbol::Type::IBase*>& t_arguments
    ) -> Expected<Symbol::IBase*>
    {
        ACE_TRY(symbolsInstantiationResult, t_template->InstantiateSymbols(
            t_implArguments,
            t_arguments
        ));

        const bool areArgumentsPlaceholders = AreArgumentsPlaceholders(
            t_implArguments,
            t_arguments
        );
        if (!areArgumentsPlaceholders)
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
                    [&](const std::shared_ptr<const Node::IBase>& t_ast)
                    {
                        templateASTsPair.first->InstantiateSemanticsForSymbols(t_ast);
                    }
                );
            }
        }
    }
}
