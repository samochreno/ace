#include "TemplateInstantiator.hpp"

#include "Symbol/Template/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Error.hpp"

namespace Ace
{
    TemplateInstantiator::TemplateInstantiator()
        : m_Symbols{}, m_SymbolOnlyInstantiatedASTsMap{}
    {
    }

    TemplateInstantiator::~TemplateInstantiator()
    {
    }

    auto TemplateInstantiator::InstantiatePlaceholderSymbols() const -> Expected<void>
    {
        return TransformExpectedVector(m_Symbols,
        [](Symbol::Template::IBase* const t_symbol) -> Expected<void>
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

            ACE_TRY(symbolsInstantiationResult, t_symbol->InstantiateSymbols(
                upcastedImplParameters,
                upcastedParameters
            ));
            t_symbol->SetPlaceholderSymbol(symbolsInstantiationResult.Symbol);

            return ExpectedVoid;
        });
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
        m_SymbolOnlyInstantiatedASTsMap[t_template].push_back(
            symbolsInstantiationResult.AST
        );
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
