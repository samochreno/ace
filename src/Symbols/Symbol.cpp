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
#include "SrcLocation.hpp"
#include "Name.hpp"

namespace Ace
{
    auto ISymbol::GetCompilation() const -> Compilation*
    {
        return GetScope()->GetCompilation();
    }

    static auto UnwrapAlias(const ISymbol* symbol) -> const ISymbol*
    {
        auto* const typeSymbol = dynamic_cast<const ITypeSymbol*>(symbol);
        if (!typeSymbol)
        {
            return symbol;
        }

        return typeSymbol->GetUnaliased();
    }

    static auto CreateTemplateArgsSignature(
        const ITemplatableSymbol* const symbol
    ) -> std::string
    {
        std::string signature{};

        const auto implTemplateArgs = symbol->CollectImplTemplateArgs();
        if (!implTemplateArgs.empty())
        {
            signature += "<";

            bool isFirstImplTemplateArg = true;
            std::for_each(begin(implTemplateArgs), end(implTemplateArgs),
            [&](const ITypeSymbol* const implTemplateArg)
            {
                if (isFirstImplTemplateArg)
                {
                    isFirstImplTemplateArg = false;
                }
                else
                {
                    signature += ", ";
                }

                signature += implTemplateArg->CreateSignature();
            });

            signature += ">";
        }

        const auto templateArgs = symbol->CollectTemplateArgs();
        if (!templateArgs.empty())
        {
            signature += "[";

            bool isFirstTemplateArg = true;
            std::for_each(begin(templateArgs), end(templateArgs),
            [&](const ITypeSymbol* const templateArg)
            {
                if (isFirstTemplateArg)
                {
                    isFirstTemplateArg = false;
                }
                else
                {
                    signature += ", ";
                }

                signature += templateArg->CreateSignature();
            });

            signature += "]";
        }

        return signature;
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
            signature += CreateTemplateArgsSignature(templatableSymbol);
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
        std::for_each(rbegin(scopes) + 1, rend(scopes),
        [&](const std::shared_ptr<Scope>& scope)
        {
            if (isFirstScope)
            {
                signature = scope->GetName();
                isFirstScope = false;
            }
            else
            {
                signature = signature + "::" + scope->GetName();
            }
        });

        const std::string lastSeparator = [&]() -> std::string
        {
            if (GetCategory() == SymbolCategory::Instance)
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
        const SrcLocation& srcLocation
    ) const -> SymbolName
    {
        auto* const symbol = UnwrapAlias(this);
        if (symbol != this)
        {
            return symbol->CreateFullyQualifiedName(srcLocation);
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
            [&](const std::shared_ptr<Scope>& scope)
            {
                return SymbolNameSection
                {
                    Ident
                    {
                        srcLocation,
                        scope->GetName(),
                    }
                };
            }
        );

        nameSections.emplace_back(Ident{
            srcLocation,
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
                [&](ITypeSymbol* const implTemplateArg)
                {
                    return implTemplateArg->CreateFullyQualifiedName(
                        srcLocation
                    );
                }
            );

            if (!implTemplateArgNames.empty())
            {
                (rbegin(nameSections) + 1)->TemplateArgs = implTemplateArgNames;
            }

            const auto templateArgs = templatableSymbol->CollectTemplateArgs();
            std::vector<SymbolName> templateArgNames{};
            std::transform(
                begin(templateArgs), 
                end  (templateArgs), 
                back_inserter(templateArgNames),
                [&](ITypeSymbol* const templateArg)
                {
                    return templateArg->CreateFullyQualifiedName(
                        srcLocation
                    );
                }
            );

            if (!templateArgNames.empty())
            {
                rbegin(nameSections)->TemplateArgs = templateArgNames;
            }
        }

        return SymbolName
        {
            nameSections,
            SymbolNameResolutionScope::Global,
        };
    }

    static auto IsTypeError(const ITypeSymbol* const typeSymbol) -> bool
    {
        auto* const compilation = typeSymbol->GetCompilation();

        if (compilation->GetErrorSymbols().Contains(typeSymbol))
        {
            return true;
        }

        const auto implTemplateArgs = typeSymbol->CollectImplTemplateArgs();
        const auto errorImplTemplateArgIt = std::find_if(
            begin(implTemplateArgs),
            end  (implTemplateArgs),
            [&](const ITypeSymbol* const implTemplateArg)
            {
                return IsTypeError(implTemplateArg);
            }
        );
        if (errorImplTemplateArgIt != end(implTemplateArgs))
        {
            return true;
        }

        const auto templateArgs = typeSymbol->CollectTemplateArgs();
        const auto errorTemplateArgIt = std::find_if(
            begin(templateArgs),
            end  (templateArgs),
            [&](const ITypeSymbol* const templateArg)
            {
                return IsTypeError(templateArg);
            }
        );
        if (errorTemplateArgIt != end(templateArgs))
        {
            return true;
        }

        return false;
    }

    auto ISymbol::IsError() const -> bool
    {
        if (GetCompilation()->GetErrorSymbols().Contains(this))
        {
            return true;
        }

        if (const auto* const type = dynamic_cast<const ITypeSymbol*>(this))
        {
            return IsTypeError(type);
        }

        return false;
    }
}
