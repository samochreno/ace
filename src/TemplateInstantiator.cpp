#include "TemplateInstantiator.hpp"

#include "Symbol/Template/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/TemplateParam/Impl.hpp"
#include "Symbol/Type/TemplateParam/Normal.hpp"
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
            const auto implParams = t_symbol->CollectImplParams();
            std::vector<Symbol::Type::IBase*> upcastedImplParams
            {
                begin(implParams),
                end  (implParams),
            };

            const auto params = t_symbol->CollectParams();
            std::vector<Symbol::Type::IBase*> upcastedParams
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

            return Void;
        });
    }

    static auto IsArgPlaceholder(
        Symbol::Type::IBase* const t_arg
    ) -> bool;

    static auto AreArgsPlaceholders(
        const std::vector<Symbol::Type::IBase*>& t_implArgs,
        const std::vector<Symbol::Type::IBase*>& t_args
    ) -> bool
    {
        const auto foundImplIt = std::find_if(
            begin(t_implArgs),
            end  (t_implArgs),
            [](Symbol::Type::IBase* const t_implArg)
            {
                return IsArgPlaceholder(t_implArg);
            }
        );
        if (foundImplIt != end(t_implArgs))
            return true;

        const auto foundIt = std::find_if(
            begin(t_args),
            end  (t_args),
            [](Symbol::Type::IBase* const t_arg)
            {
                return IsArgPlaceholder(t_arg);
            }
        );
        if (foundIt != end(t_args))
            return true;

        return false;
    }

    static auto IsArgPlaceholder(
        Symbol::Type::IBase* t_arg
    ) -> bool
    {
        t_arg = t_arg->GetUnaliased();

        if (dynamic_cast<Symbol::Type::TemplateParam::Impl*>(t_arg))
            return true;

        if (dynamic_cast<Symbol::Type::TemplateParam::Normal*>(t_arg))
            return true;

        auto* const templatable =
            dynamic_cast<Symbol::ITemplatable*>(t_arg);
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
        Symbol::Template::IBase* const t_template,
        const std::vector<Symbol::Type::IBase*>& t_implArgs,
        const std::vector<Symbol::Type::IBase*>& t_args
    ) -> Expected<Symbol::IBase*>
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
                    [&](const std::shared_ptr<const Node::IBase>& t_ast)
                    {
                        templateASTsPair.first->InstantiateSemanticsForSymbols(t_ast);
                    }
                );
            }
        }
    }
}
