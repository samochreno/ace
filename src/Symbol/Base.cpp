#include "Symbol/Base.hpp"

#include <vector>
#include <string>
#include <unordered_set>

#include "Asserts.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Templatable.hpp"
#include "Scope.hpp"
#include "Name.hpp"

namespace Ace::Symbol
{
    static auto UnwrapAlias(const Symbol::IBase* const t_symbol) -> const Symbol::IBase*
    {
        auto* const typeSymbol = dynamic_cast<const Symbol::Type::IBase*>(t_symbol);
        if (!typeSymbol)
        {
            return t_symbol;
        }

        const auto* const symbol = typeSymbol ? 
            dynamic_cast<const Symbol::IBase*>(typeSymbol->GetUnaliased()) : 
            t_symbol;

        ACE_ASSERT(symbol);
        return symbol;
    }

    auto IBase::CreatePartialSignature() const -> std::string
    {
        auto* const symbol = UnwrapAlias(this);
        if (symbol != this)
        {
            return symbol->CreatePartialSignature();
        }

        std::string signature = GetName();

        if (auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(this))
        {
            const auto templateArguments = templatableSymbol->CollectTemplateArguments();

            if (!templateArguments.empty())
            {
                signature += "[";

                std::for_each(begin(templateArguments), end(templateArguments), [&]
                (const Symbol::Type::IBase* const t_templateArgument)
                {
                    signature += t_templateArgument->CreateSignature();
                });

                signature += "]";
            }
        }

        return signature;
    }

    auto IBase::CreateSignature() const -> std::string
    {
        auto* const symbol = UnwrapAlias(this);
        if (symbol != this)
        {
            return symbol->CreateSignature();
        }

        auto* scope = GetScope();
        std::vector<Scope*> scopes{};
        while (scope)
        {
            scopes.push_back(scope);
            scope = scope->GetParent();
        }

        std::string signature{};
        bool isFirstScope = true;
        std::for_each(rbegin(scopes), rend(scopes), [&]
        (Scope* const t_scope)
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
            if (IsInstance())
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

    auto IBase::GetFullyQualifiedName() const -> Name::Symbol::Full
    {
        auto* const symbol = UnwrapAlias(this);
        if (symbol != this)
        {
            return symbol->GetFullyQualifiedName();
        }

        std::vector<Name::Symbol::Section> nameSections{};

        std::vector<Scope*> scopes{};
        for (Scope* scope = GetScope(); scope; scope = scope->GetParent())
        {
            scopes.push_back(scope);
        }

        std::transform(rbegin(scopes) + 1, rend(scopes), back_inserter(nameSections), []
        (Scope* const t_scope)
        {
            return Name::Symbol::Section{ t_scope->GetName() };
        });

        nameSections.emplace_back(GetName());

        if (auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(this))
        {
            const auto implTemplateArguments = templatableSymbol->CollectImplTemplateArguments();
            std::vector<Name::Symbol::Full> implTemplateArgumentNames{};
            std::transform(begin(implTemplateArguments), end(implTemplateArguments), back_inserter(implTemplateArgumentNames), []
            (Symbol::Type::IBase* const t_implTemplateArgument)
            {
                return t_implTemplateArgument->GetFullyQualifiedName();
            });

            (rbegin(nameSections) + 1)->TemplateArguments = implTemplateArgumentNames;

            const auto templateArguments = templatableSymbol->CollectTemplateArguments();
            std::vector<Name::Symbol::Full> templateArgumentNames{};
            std::transform(begin(templateArguments), end(templateArguments), back_inserter(templateArgumentNames), []
            (Symbol::Type::IBase* const t_templateArgument)
            {
                return t_templateArgument->GetFullyQualifiedName();
            });

            rbegin(nameSections)->TemplateArguments = templateArgumentNames;
        }

        return Name::Symbol::Full{ nameSections, true };
    }
}
