#include "Symbols/Symbol.hpp"

#include <vector>
#include <string>
#include <unordered_set>
#include <optional>

#include "Assert.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/TemplatableSymbol.hpp"
#include "Scope.hpp"
#include "SourceLocation.hpp"
#include "Name.hpp"

namespace Ace
{
    auto ISymbol::GetCompilation() const -> const Compilation*
    {
        return GetScope()->GetCompilation();
    }

    static auto UnwrapAlias(const ISymbol* const t_symbol) -> const ISymbol*
    {
        auto* const typeSymbol = dynamic_cast<const ITypeSymbol*>(t_symbol);
        if (!typeSymbol)
        {
            return t_symbol;
        }

        const auto* const symbol = typeSymbol ? 
            dynamic_cast<const ISymbol*>(typeSymbol->GetUnaliased()) : 
            t_symbol;

        ACE_ASSERT(symbol);
        return symbol;
    }

    auto ISymbol::CreatePartialSignature() const -> std::string
    {
        auto* const symbol = UnwrapAlias(this);
        if (symbol != this)
        {
            return symbol->CreatePartialSignature();
        }

        std::string signature = GetName().String;

        if (auto* const templatableSymbol = dynamic_cast<const ITemplatableSymbol*>(this))
        {
            const auto templateArgs = templatableSymbol->CollectTemplateArgs();

            if (!templateArgs.empty())
            {
                signature += "[";

                std::for_each(begin(templateArgs), end(templateArgs),
                [&](const ITypeSymbol* const t_templateArg)
                {
                    signature += t_templateArg->CreateSignature();
                });

                signature += "]";
            }
        }

        return signature;
    }

    auto ISymbol::CreateSignature() const -> std::string
    {
        auto* const symbol = UnwrapAlias(this);
        if (symbol != this)
        {
            return symbol->CreateSignature();
        }

        std::vector<std::shared_ptr<Scope>> scopes{};
        for (
            auto optScope = std::optional{ GetScope() };
            optScope.has_value();
            optScope = optScope.value()->GetParent()
            )
        {
            scopes.push_back(optScope.value());
        }

        std::string signature{};
        bool isFirstScope = true;
        std::for_each(rbegin(scopes), rend(scopes),
        [&](const std::shared_ptr<Scope>& t_scope)
        {
            if (isFirstScope)
            {
                signature = t_scope->GetName();
                isFirstScope = false;
            }
            else
            {
                signature = signature + "::" + t_scope->GetName();
            }
        });

        const std::string lastSeparator = [&]() -> std::string
        {
            if (GetSymbolCategory() == SymbolCategory::Instance)
            {
                return ".";
            }
            else
            {
                return "::";
            }
        }();

        signature = signature + lastSeparator + CreatePartialSignature();
        return signature;
    }

    auto ISymbol::CreateFullyQualifiedName(
        const SourceLocation& t_sourceLocation
    ) const -> SymbolName
    {
        auto* const symbol = UnwrapAlias(this);
        if (symbol != this)
        {
            return symbol->CreateFullyQualifiedName(t_sourceLocation);
        }

        std::vector<SymbolNameSection> nameSections{};

        std::vector<std::shared_ptr<Scope>> scopes{};
        for (
            auto optScope = std::optional{ GetScope() };
            optScope.has_value();
            optScope = optScope.value()->GetParent()
            )
        {
            scopes.push_back(optScope.value());
        }

        std::transform(
            rbegin(scopes) + 1, 
            rend  (scopes), 
            back_inserter(nameSections),
            [&](const std::shared_ptr<Scope>& t_scope)
            {
                return SymbolNameSection
                {
                    Identifier
                    {
                        t_sourceLocation,
                        t_scope->GetName(),
                    }
                };
            }
        );

        nameSections.emplace_back(Identifier{
            t_sourceLocation,
            GetName().String
        });

        if (const auto* const templatableSymbol = dynamic_cast<const ITemplatableSymbol*>(this))
        {
            const auto implTemplateArgs = templatableSymbol->CollectImplTemplateArgs();
            std::vector<SymbolName> implTemplateArgNames{};
            std::transform(
                begin(implTemplateArgs), 
                end  (implTemplateArgs), 
                back_inserter(implTemplateArgNames),
                [&](ITypeSymbol* const t_implTemplateArg)
                {
                    return t_implTemplateArg->CreateFullyQualifiedName(
                        t_sourceLocation
                    );
                }
            );

            (rbegin(nameSections) + 1)->TemplateArgs = implTemplateArgNames;

            const auto templateArgs = templatableSymbol->CollectTemplateArgs();
            std::vector<SymbolName> templateArgNames{};
            std::transform(
                begin(templateArgs), 
                end  (templateArgs), 
                back_inserter(templateArgNames),
                [&](ITypeSymbol* const t_templateArg)
                {
                    return t_templateArg->CreateFullyQualifiedName(
                        t_sourceLocation
                    );
                }
            );

            rbegin(nameSections)->TemplateArgs = templateArgNames;
        }

        return SymbolName
        {
            nameSections,
            SymbolNameResolutionScope::Global,
        };
    }
}
