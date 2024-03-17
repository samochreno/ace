#include "Symbols/Symbol.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Assert.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/Aliases/AliasTypeSymbol.hpp"
#include "Symbols/PrototypeSymbol.hpp"
#include "Symbols/GenericSymbol.hpp"
#include "Symbols/Types/TypeParamTypeSymbol.hpp"
#include "Symbols/Types/TraitSelfTypeSymbol.hpp"
#include "Symbols/Types/Aliases/ImplSelfAliasTypeSymbol.hpp"
#include "Scope.hpp"
#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Compilation.hpp"
#include "Keyword.hpp"

namespace Ace
{
    auto ISymbol::GetCompilation() const -> Compilation*
    {
        return GetScope()->GetCompilation();
    }

    auto ISymbol::GetUnaliased() const -> ISymbol*
    {
        auto* aliasType =
            dynamic_cast<IAliasTypeSymbol*>(const_cast<ISymbol*>(this));
        if (!aliasType)
        {
            return const_cast<ISymbol*>(this);
        }

        auto* type = dynamic_cast<ITypeSymbol*>(aliasType);
        while ((aliasType = dynamic_cast<IAliasTypeSymbol*>(type)))
        {
            type = aliasType->GetAliasedType();
        }

        return type;
    }

    static auto CreateTypeArgsSignature(
        const IGenericSymbol* const generic
    ) -> std::string
    {
        const auto& args = generic->GetTypeArgs();

        if (args.empty())
        {
            return {};
        }

        std::string signature{};

        signature += "[";


        bool isFirstArg = true;
        std::for_each(begin(args), end(args),
        [&](ITypeSymbol* const arg)
        {
            if (isFirstArg)
            {
                isFirstArg = false;
            }
            else
            {
                signature += ", ";
            }

            signature += arg->CreateSignature();
        });

        signature += "]";

        return signature;
    }

    auto ISymbol::CreateLocalSignature() const -> std::string
    {
        auto* const symbol = GetUnaliased();
        if (symbol != this)
        {
            return symbol->CreateLocalSignature();
        }

        std::string signature = GetName().String;

        if (auto* const prototype = dynamic_cast<const PrototypeSymbol*>(this))
        {
            signature += "<" + prototype->GetSelfType()->CreateSignature() + ">";
        }

        if (auto* const generic = dynamic_cast<const IGenericSymbol*>(this))
        {
            signature += CreateTypeArgsSignature(generic);
        }

        return signature;
    }

    static auto CreateScopeName(
        const std::shared_ptr<Scope>& scope
    ) -> std::string
    {
        return scope->GetName().has_value() ?
            scope->GetName().value() :
            scope->GetAnonymousName().value();
    }

    auto ISymbol::CreateSignature() const -> std::string
    {
        auto* const symbol = GetUnaliased();
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
                signature = CreateScopeName(scope);
                isFirstScope = false;
            }
            else
            {
                signature = signature + "::" + CreateScopeName(scope);
            }
        });

        return signature.empty() ?
            CreateLocalSignature() :
            signature + "::" + CreateLocalSignature();
    }

    auto ISymbol::CreateFullyQualifiedName(
        const SrcLocation& srcLocation
    ) const -> SymbolName
    {
        auto* const symbol = GetUnaliased();
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
                    Ident{ srcLocation, CreateScopeName(scope) }
                };
            }
        );

        nameSections.emplace_back(Ident{ srcLocation, GetName().String });

        auto* const generic = dynamic_cast<const IGenericSymbol*>(this);

        if (generic && !generic->GetTypeArgs().empty())
        {
            const auto& args = generic->GetTypeArgs();

            auto& lastArgNames = nameSections.back().TypeArgs;
            lastArgNames.clear();

            std::transform(begin(args), end(args), back_inserter(lastArgNames),
            [&](ITypeSymbol* const arg)
            {
                return arg->CreateFullyQualifiedName(srcLocation);
            });
        }

        return SymbolName{ nameSections, SymbolNameResolutionScope::Global };
    }

    static auto CreateScopeDisplayName(
        const std::shared_ptr<Scope>& scope
    ) -> std::string
    {
        return scope->GetName().has_value() ? scope->GetName().value() : "...";
    }

    auto ISymbol::CreateLocalDisplayName() const -> std::string
    {
        std::string signature{};

        signature += GetName().String;

        const auto* const genericSymbol =
            dynamic_cast<const IGenericSymbol*>(this);

        const auto typeArgs = genericSymbol->GetTypeArgs();
        if (!typeArgs.empty())
        {
            signature += "[";

            bool isFirstTypeArg = true;
            std::for_each(begin(typeArgs), end(typeArgs),
            [&](ITypeSymbol* const arg)
            {
                if (isFirstTypeArg)
                {
                    isFirstTypeArg = false;
                }
                else
                {
                    signature += ", ";
                }

                signature += arg->CreateDisplayName();
            });

            signature += "]";
        }

        return signature;
    }

    auto ISymbol::CreateDisplayName() const -> std::string
    {
        auto* const implSelfAliasTypeSymbol =
            dynamic_cast<const ImplSelfAliasTypeSymbol*>(this);
        if (implSelfAliasTypeSymbol)
        {
            return implSelfAliasTypeSymbol->GetUnaliased()->CreateDisplayName();
        }

        auto* const typeSymbol =
            dynamic_cast<ITypeSymbol*>(const_cast<ISymbol*>(this));
        if (typeSymbol)
        {
            if (typeSymbol->IsRef())
            {
                return "&" + typeSymbol->GetWithoutRef()->CreateDisplayName();
            }

            if (typeSymbol->IsWeakPtr())
            {
                return "~" + typeSymbol->GetWithoutWeakPtr()->CreateDisplayName();
            }

            if (typeSymbol->IsAnyStrongPtr())
            {
                return "*" + typeSymbol->GetWithoutStrongPtr()->CreateDisplayName();
            }
        }

        const auto tokenKind = [&]() -> TokenKind
        {
            if (this == GetCompilation()->GetVoidTypeSymbol())
            {
                return TokenKind::VoidKeyword;
            }

            const auto& natives = GetCompilation()->GetNatives();

            if (this == natives.Int8.GetSymbol())
            {
                return TokenKind::Int8Keyword;
            }

            if (this == natives.Int16.GetSymbol())
            {
                return TokenKind::Int16Keyword;
            }

            if (this == natives.Int32.GetSymbol())
            {
                return TokenKind::Int32Keyword;
            }

            if (this == natives.Int64.GetSymbol())
            {
                return TokenKind::Int64Keyword;
            }

            if (this == natives.UInt8.GetSymbol())
            {
                return TokenKind::UInt8Keyword;
            }

            if (this == natives.UInt16.GetSymbol())
            {
                return TokenKind::UInt16Keyword;
            }

            if (this == natives.UInt32.GetSymbol())
            {
                return TokenKind::UInt32Keyword;
            }

            if (this == natives.UInt64.GetSymbol())
            {
                return TokenKind::UInt64Keyword;
            }

            if (this == natives.Int.GetSymbol())
            {
                return TokenKind::IntKeyword;
            }

            if (this == natives.Float32.GetSymbol())
            {
                return TokenKind::Float32Keyword;
            }

            if (this == natives.Float64.GetSymbol())
            {
                return TokenKind::Float64Keyword;
            }

            if (this == natives.Bool.GetSymbol())
            {
                return TokenKind::BoolKeyword;
            }

            return TokenKind::Ident;
        }();
        
        if (tokenKind != TokenKind::Ident)
        {
            return std::string{ TokenKindToKeywordMap.at(tokenKind) };
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

        size_t startScopeIndex = 1;
        if (
            (rbegin(scopes) + 1)->get() ==
            GetCompilation()->GetPackageBodyScope().get()
            )
        {
            startScopeIndex++;
        }

        std::string signature{};
        bool isFirstScope = true;
        std::for_each(rbegin(scopes) + startScopeIndex, rend(scopes),
        [&](const std::shared_ptr<Scope>& scope)
        {
            if (isFirstScope)
            {
                signature = CreateScopeDisplayName(scope);
                isFirstScope = false;
            }
            else
            {
                signature = signature + "::" + CreateScopeDisplayName(scope);
            }
        });

        return signature.empty() ?
            CreateLocalDisplayName() :
            signature + "::" + CreateLocalDisplayName();
    }

    static auto IsTypeError(const ITypeSymbol* const type) -> bool
    {
        auto* const compilation = type->GetCompilation();

        if (compilation->GetErrorSymbols().Contains(type))
        {
            return true;
        }

        const auto errorTypeArgIt = std::find_if(
            begin(type->GetTypeArgs()),
            end  (type->GetTypeArgs()),
            IsTypeError
        );
        if (errorTypeArgIt != end(type->GetTypeArgs()))
        {
            return true;
        }

        return false;
    }

    auto ISymbol::IsError() const -> bool
    {
        auto* const symbol = GetUnaliased();
        if (symbol != this)
        {
            return symbol->IsError();
        }

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

    auto ISymbol::GetRoot() const -> ISymbol*
    {
        auto* const generic =
            dynamic_cast<const IGenericSymbol*>(GetUnaliased());

        if (generic && generic->IsInstance())
        {
            return generic->GetGenericRoot();
        }

        return const_cast<ISymbol*>(this)->GetUnaliased();
    }

    static auto CreateInstantiatedSymbol(
        const IGenericSymbol* const symbol,
        const InstantiationContext& context
    ) -> IGenericSymbol*
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        auto* const mutSymbol = dynamic_cast<IGenericSymbol*>(
            const_cast<IGenericSymbol*>(symbol)->GetUnaliased()
        );
        ACE_ASSERT(mutSymbol);

        auto* const traitSelfType =
            dynamic_cast<TraitSelfTypeSymbol*>(mutSymbol);
        if (traitSelfType)
        {
            return context.OptSelfType.value_or(traitSelfType);
        }

        if (!symbol->IsPlaceholder())
        {
            return mutSymbol;
        }

        auto* const typeParam = dynamic_cast<TypeParamTypeSymbol*>(mutSymbol);
        if (typeParam)
        {
            return context.TypeArgs.at(typeParam->GetIndex());
        }

        std::vector<ITypeSymbol*> instantiatedTypeArgs{};
        std::transform(
            begin(symbol->GetTypeArgs()),
            end  (symbol->GetTypeArgs()),
            back_inserter(instantiatedTypeArgs),
            [&](ITypeSymbol* const arg)
            {
                return CreateInstantiated<ITypeSymbol>(arg, context);
            }
        );

        std::optional<ITypeSymbol*> optSelfType{};
        if (auto* const prototype = dynamic_cast<PrototypeSymbol*>(mutSymbol))
        {
            optSelfType = CreateInstantiated<ITypeSymbol>(
                prototype->GetSelfType(),
                context
            );
        }

        auto* const instantiated = Scope::ForceCollectGenericInstance(
            symbol->GetGenericRoot(),
            instantiatedTypeArgs,
            std::nullopt,
            optSelfType
        );
        auto* const castedInstantiated =
            dynamic_cast<IGenericSymbol*>(instantiated);
        ACE_ASSERT(castedInstantiated);
        return castedInstantiated;
    }

    auto CreateUnaliasedInstantiatedSymbol(
        const IGenericSymbol* const symbol,
        const InstantiationContext& context
    ) -> IGenericSymbol*
    {
        auto* const instantiated = CreateInstantiatedSymbol(symbol, context);
        
        auto* const castedInstantiated =
            dynamic_cast<IGenericSymbol*>(instantiated->GetUnaliased());
        ACE_ASSERT(castedInstantiated);
        return castedInstantiated;
    }
}
