#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <algorithm>
#include <iterator>
#include <unordered_map>

#include "Diagnostic.hpp"
#include "Diagnostics/SymbolTypeDiagnostics.hpp"
#include "Name.hpp"
#include "SymbolCategory.hpp"
#include "Ident.hpp"
#include "GenericInstantiator.hpp"

namespace Ace
{
    class Scope;

    class Compilation;
    class IDecl;

    class ISymbol;
    class ModSymbol;
    class ITypeSymbol;
    class TraitTypeSymbol;
    class InherentImplSymbol;
    class TraitImplSymbol;
    class IGenericSymbol;
    class TypeParamTypeSymbol;

    auto DiagnoseInaccessibleSymbol(
        const SrcLocation& srcLocation,
        ISymbol* const symbol,
        const std::shared_ptr<const Scope>& beginScope
    ) -> Diagnosed<void>;

    auto GetUnaliasedSymbol(ISymbol* const symbol) -> ISymbol*;

    template<typename TSymbol>
    auto IsCorrectSymbolType(
        const SrcLocation& srcLocation,
        ISymbol* const symbol
    ) -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (dynamic_cast<TSymbol*>(symbol))
        {
            return Void{ std::move(diagnostics) };
        }

        if (dynamic_cast<TSymbol*>(GetUnaliasedSymbol(symbol)))
        {
            return Void{ std::move(diagnostics) };
        }

        diagnostics.Add(CreateIncorrectSymbolTypeError<TSymbol>(srcLocation));
        return std::move(diagnostics);
    }

    template<typename TSymbol>
    auto GetOrCastToCorrectSymbolType(ISymbol* const symbol) -> TSymbol*
    {
        if (auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol))
        {
            return castedSymbol;
        }

        auto* const unaliasedSymbol =
            dynamic_cast<TSymbol*>(GetUnaliasedSymbol(symbol));
        ACE_ASSERT(unaliasedSymbol);

        return unaliasedSymbol;
    }

    auto IsCorrectSymbolCategory(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol,
        const SymbolCategory symbolCategory
    ) -> Expected<void>;

    auto CreateSymbolRedeclarationError(
        const ISymbol* const originalSymbol,
        const ISymbol* const redeclaredSymbol
    ) -> DiagnosticGroup;

    auto CastToGeneric(
        const ISymbol* const symbol
    ) -> const IGenericSymbol*;
    auto CastToGeneric(
        const ITypeSymbol* const symbol
    ) -> const IGenericSymbol*;
    auto GetTypeArgs(
        const IGenericSymbol* const generic
    ) -> const std::vector<ITypeSymbol*>&;

    auto GetDerefed(ITypeSymbol* const type) -> ITypeSymbol*;

    auto GetPrototypeSelfType(
        const ISymbol* const symbol
    ) -> std::optional<ITypeSymbol*>;

    struct SymbolResolutionContext
    {
        auto IsLastNameSection() const -> bool;
        auto GetName() const -> const std::string&;

        SrcLocation SrcLocation{};
        std::shared_ptr<const Scope> BeginScope{};
        std::vector<SymbolNameSection>::const_iterator NameSectionsBegin{};
        std::vector<SymbolNameSection>::const_iterator NameSectionsEnd{};
        std::vector<SymbolNameSection>::const_iterator NameSection{};
        std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>> OptArgTypes{};
        std::function<Expected<void>(const Ace::SrcLocation&, ISymbol*)> IsCorrectSymbolType{};
        std::vector<std::shared_ptr<const Scope>> Scopes{};
        size_t SuppliedTypeArgCount{};
        std::vector<ITypeSymbol*> TypeArgs{};
        bool IsRoot{};
        std::optional<ITypeSymbol*> OptSelfType{};
    };

    class GlobalScope
    {
    public:
        GlobalScope();
        GlobalScope(Compilation* const compilation);
        ~GlobalScope();

        auto Unwrap() const -> const std::shared_ptr<Scope>&;

    private:
        std::shared_ptr<Scope> m_Scope{};
    };

    class Scope : public std::enable_shared_from_this<Scope>
    {
        friend GlobalScope;

    public:
        ~Scope();
        
        auto GetCompilation() const -> Compilation*;
        auto GetNestLevel() const -> size_t;
        auto GetParent() const -> const std::optional<std::shared_ptr<Scope>>&;
        auto GetName()          const -> const std::optional<std::string>&;
        auto GetAnonymousName() const -> const std::optional<std::string>&;
        auto GetGenericInstantiator() -> GenericInstantiator&;

        auto FindMod() const -> std::optional<ModSymbol*>;
        auto FindPackageMod() const -> ModSymbol*;

        auto CreateChild() -> std::shared_ptr<Scope>;
        auto GetOrCreateChild(
            const std::string& name
        ) -> std::shared_ptr<Scope>;
        auto HasChild(const std::shared_ptr<const Scope>& scope) const -> bool;
        auto CollectChildren() const -> std::vector<std::shared_ptr<Scope>>;

        auto HasSymbolWithName(const std::string& name) const -> bool;

        template<typename TSymbol>
        static auto DeclareSymbol(
            std::unique_ptr<TSymbol> ownedSymbol
        ) -> Diagnosed<TSymbol*>
        {
            auto diagnostics = DiagnosticBag::Create();

            auto* const symbol = ownedSymbol.get();
            const auto scope = symbol->GetScope();

            auto* const generic = CastToGeneric(symbol);

            std::vector<ITypeSymbol*> typeArgs{};
            if (generic)
            {
                typeArgs = GetTypeArgs(generic);
            }

            const auto optSameSymbol = scope->GetDeclaredSymbol(
                symbol->GetName().String,
                typeArgs,
                GetPrototypeSelfType(symbol)
            );

            if (optSameSymbol.has_value())
            {
                diagnostics.Add(CreateSymbolRedeclarationError(
                    optSameSymbol.value(),
                    symbol
                ));

                auto* const sameSymbol = optSameSymbol.value();

                auto* const castedSameSymbol =
                    dynamic_cast<TSymbol*>(sameSymbol);

                const bool isSameType = castedSameSymbol != nullptr;
                if (isSameType)
                {
                    return Diagnosed
                    {
                        castedSameSymbol,
                        std::move(diagnostics),
                    };
                }
            }

            scope->m_SymbolMap[symbol->GetName().String].push_back(
                std::move(ownedSymbol)
            );
            scope->OnSymbolDeclared(symbol);

            return Diagnosed{ symbol, std::move(diagnostics) };
        }
        static auto DeclareSymbol(
            const IDecl* const decl
        ) -> Diagnosed<ISymbol*>;
        static auto RemoveSymbol(ISymbol* const symbol) -> void;

        static auto CreateArgTypes(
            const std::vector<ITypeSymbol*>& argTypes
        ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>;

        template<typename T>
        auto ResolveStaticSymbol(const Ident& name) const -> Expected<T*>
        {
            return ResolveStaticSymbol<T>(name, {});
        }
        template<typename T>
        auto ResolveStaticSymbol(
            const Ident& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<T*>
        {

            const SymbolName symbolName
            {
                SymbolNameSection{ name },
                SymbolNameResolutionScope::Local,
            };

            return ResolveStaticSymbol<T>(symbolName, optArgTypes);
        }
        template<typename T>
        auto ResolveStaticSymbol(const SymbolName& name) const -> Expected<T*>
        {
            return ResolveStaticSymbol<T>(name, {});
        }
        template<typename T>
        auto ResolveStaticSymbol(
            const SymbolName& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<T*>
        {
            auto diagnostics = DiagnosticBag::Create();

            const auto srcLocation = name.Sections.front().CreateSrcLocation();

            const auto optBeginScope = diagnostics.Collect(
                FindStaticBeginScope(srcLocation, name)
            );
            if (!optBeginScope.has_value())
            {
                return std::move(diagnostics);
            }

            const auto optSymbol = diagnostics.Collect(ResolveSymbolInScopes({
                srcLocation,
                shared_from_this(),
                begin(name.Sections),
                end  (name.Sections),
                begin(name.Sections),
                optArgTypes,
                IsCorrectSymbolType<T>,
                std::vector{ optBeginScope.value() },
                begin(name.Sections)->TypeArgs.size(),
                {},
                false,
                std::nullopt
            }));
            if (!optSymbol.has_value())
            {
                return std::move(diagnostics);
            }

            auto* const symbol =
                GetOrCastToCorrectSymbolType<T>(optSymbol.value());

            const auto isCorrectSymbolCategory = diagnostics.Collect(
                IsCorrectSymbolCategory(
                    srcLocation,
                    symbol,
                    SymbolCategory::Static
                )
            );
            if (!isCorrectSymbolCategory)
            {
                return std::move(diagnostics);
            }

            return Expected{ symbol, std::move(diagnostics) };
        }

        template<typename T>
        auto ResolveInstanceSymbol(
            ITypeSymbol* const selfType,
            const SymbolNameSection& name
        ) const -> Expected<T*>
        {
            return ResolveInstanceSymbol<T>(selfType, name, {});
        }
        template<typename T>
        auto ResolveInstanceSymbol(
            ITypeSymbol* selfType,
            const SymbolNameSection& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<T*>
        {
            auto diagnostics = DiagnosticBag::Create();

            selfType = GetDerefed(selfType);

            const std::vector nameSections{ name };

            const auto srcLocation = nameSections.front().CreateSrcLocation();

            std::optional<ISymbol*> optSymbol{};

            const auto selfTypeArgs = GetTypeArgs(CastToGeneric(selfType));

            const auto suppliedTypeArgCount =
                selfTypeArgs.size() +
                (begin(nameSections)->TypeArgs.size());

            const auto inherentScopes =
                CollectInherentScopes(name.Name.String, selfType);

            auto inherentDiagnostics = DiagnosticBag::Create();
            optSymbol = inherentDiagnostics.Collect(ResolveSymbolInScopes({
                srcLocation,
                shared_from_this(),
                begin(nameSections),
                end  (nameSections),
                begin(nameSections),
                optArgTypes,
                IsCorrectSymbolType<T>,
                inherentScopes,
                suppliedTypeArgCount,
                selfTypeArgs,
                false,
                selfType
            }));

            if (optSymbol.has_value())
            {
                diagnostics.Add(std::move(inherentDiagnostics));
            }
            else
            {
                const auto optTraitScopes = diagnostics.Collect(
                    CollectTraitScopes(name.Name, selfType)
                );
                if (!optTraitScopes.has_value())
                {
                    return std::move(diagnostics);
                }

                auto traitDiagnostics = DiagnosticBag::Create();
                optSymbol = traitDiagnostics.Collect(ResolveSymbolInScopes({
                    srcLocation,
                    shared_from_this(),
                    begin(nameSections),
                    end  (nameSections),
                    begin(nameSections),
                    optArgTypes,
                    IsCorrectSymbolType<T>,
                    optTraitScopes.value(),
                    suppliedTypeArgCount,
                    selfTypeArgs,
                    false,
                    selfType
                }));
                if (!optSymbol.has_value())
                {
                    diagnostics.Add(std::move(inherentDiagnostics));
                    return std::move(diagnostics);
                }

                diagnostics.Add(std::move(traitDiagnostics));
            }

            auto* const symbol =
                GetOrCastToCorrectSymbolType<T>(optSymbol.value());

            const auto isCorrectSymbolCategory = diagnostics.Collect(
                IsCorrectSymbolCategory(
                    srcLocation,
                    symbol,
                    SymbolCategory::Instance
                )
            );
            if (!isCorrectSymbolCategory)
            {
                return std::move(diagnostics);
            }

            return Expected{ symbol, std::move(diagnostics) };
        }

        template<typename T>
        auto ResolveRoot(
            const SymbolName& name
        ) const -> Expected<T*>
        {
            static_assert(std::is_base_of_v<IGenericSymbol, T>);

            auto diagnostics = DiagnosticBag::Create();

            const auto srcLocation = name.Sections.front().CreateSrcLocation();

            const auto optBeginScope = diagnostics.Collect(
                FindStaticBeginScope(srcLocation, name)
            );
            if (!optBeginScope.has_value())
            {
                return std::move(diagnostics);
            }

            const auto optSymbol = diagnostics.Collect(ResolveSymbolInScopes({
                srcLocation,
                shared_from_this(),
                begin(name.Sections),
                end  (name.Sections),
                begin(name.Sections),
                std::nullopt,
                IsCorrectSymbolType<T>,
                std::vector{ optBeginScope.value() },
                begin(name.Sections)->TypeArgs.size(),
                {},
                true,
                std::nullopt
            }));
            if (!optSymbol.has_value())
            {
                return std::move(diagnostics);
            }

            auto* const symbol =
                GetOrCastToCorrectSymbolType<T>(optSymbol.value());

            return Expected{ symbol, std::move(diagnostics) };
        }
        
        template<typename TSymbol>
        auto CollectSymbols() const -> std::vector<TSymbol*>
        {
            std::vector<TSymbol*> symbols{};

            std::for_each(begin(m_SymbolMap), end(m_SymbolMap),
            [&](const auto& pair)
            {
                std::for_each(begin(pair.second), end(pair.second),
                [&](const std::unique_ptr<ISymbol>& ownedSymbol)
                {
                    auto* const symbol =
                        dynamic_cast<TSymbol*>(ownedSymbol.get());
                    if (symbol)
                    {
                        symbols.push_back(symbol);
                    }
                });
            });

            return symbols;
        }
        template<typename TSymbol>
        auto CollectSymbolsRecursive() const -> std::vector<TSymbol*>
        {
            auto symbols = CollectSymbols<TSymbol>();

            std::for_each(begin(m_Children), end(m_Children),
            [&](const std::weak_ptr<Scope>& child)
            {
                const auto childSymbols =
                    child.lock()->CollectSymbolsRecursive<TSymbol>();

                symbols.insert(
                    end(symbols), 
                    begin(childSymbols), 
                    end  (childSymbols)
                );
            });

            return symbols;
        }

        auto CollectAllSymbols() const -> std::vector<ISymbol*>;
        auto CollectAllSymbolsRecursive() const -> std::vector<ISymbol*>;

        static auto CollectGenericInstance(
            const SrcLocation& srcLocation,
            IGenericSymbol* const root,
            const std::vector<ITypeSymbol*>& args,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes = {},
            const std::optional<ITypeSymbol*>& optSelfType = {}
        ) -> Expected<ISymbol*>;
        static auto ForceCollectGenericInstance(
            IGenericSymbol* const root,
            const std::vector<ITypeSymbol*>& knownArgs,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes = {},
            const std::optional<ITypeSymbol*>& optSelfType = {}
        ) -> ISymbol*;

        auto CollectTypeParams() const -> std::vector<TypeParamTypeSymbol*>;

        static auto CollectImplOfFor(
            TraitTypeSymbol* const trait,
            ITypeSymbol* const type
        ) -> std::optional<TraitImplSymbol*>;
        static auto CollectImplOfFor(
            PrototypeSymbol* const prototype,
            ITypeSymbol* const type
        ) -> std::optional<FunctionSymbol*>;

        auto CollectConstrainedTraits(
            ITypeSymbol* const type
        ) const -> std::vector<TraitTypeSymbol*>;

        auto ResolveSelfType(
            const SrcLocation& srcLocation
        ) const -> Expected<ITypeSymbol*>;

        auto ReimportType(ITypeSymbol* const type) -> Diagnosed<ITypeSymbol*>;

    private:
        Scope(Compilation* const compilation);
        auto Clear() -> void;

        Scope(
            Compilation* const compilation,
            const std::optional<std::string>& optName,
            const std::optional<std::shared_ptr<Scope>>& optParent
        );

        auto AddChild(
            const std::optional<std::string>& optName
        ) -> std::shared_ptr<Scope>;

        auto GetDeclaredSymbol(
            const std::string& name,
            const std::vector<ITypeSymbol*>& typeArgs,
            const std::optional<ITypeSymbol*>& optSelfType
        ) const -> std::optional<ISymbol*>;

        auto OnSymbolDeclared(ISymbol* const symbol) -> void;

        static auto ResolveSymbolInScopes(
            SymbolResolutionContext context
        ) -> Expected<ISymbol*>;

        static auto CollectMatchingSymbolsInScopes(
            const SymbolResolutionContext& context
        ) -> Expected<std::vector<ISymbol*>>;
        auto CollectMatchingSymbol(
            const SymbolResolutionContext& context
        ) const -> Expected<std::optional<ISymbol*>>;

        static auto ResolveGenericInstance(
            const IGenericSymbol* const root,
            std::vector<ITypeSymbol*> typeArgs,
            const std::optional<ITypeSymbol*>& optSelfType
        ) -> std::optional<ISymbol*>;

        auto FindStaticBeginScope(
            const SrcLocation& srcLocation,
            const SymbolName& name
        ) const -> Expected<std::shared_ptr<const Scope>>;

        static auto ResolveNameSectionSymbol(
            const SymbolResolutionContext& context,
            const std::vector<ISymbol*>& matchingSymbols
        ) -> Expected<ISymbol*>;
        static auto ResolveLastNameSectionSymbol(
            const SymbolResolutionContext& context,
            const std::vector<ISymbol*>& matchingSymbols
        ) -> Expected<ISymbol*>;

        static auto CollectInherentImplFor(
            const std::string& name,
            ITypeSymbol* const type
        ) -> std::optional<InherentImplSymbol*>;
        auto CollectTraitImplFor(
            const Ident& name,
            ITypeSymbol* const type
        ) const -> Expected<std::optional<TraitImplSymbol*>>;

        static auto CollectInherentScopes(
            const std::string& name,
            ITypeSymbol* const type
        ) -> std::vector<std::shared_ptr<const Scope>>;
        auto CollectTraitScopes(
            const Ident& name,
            ITypeSymbol* const type
        ) const -> Expected<std::vector<std::shared_ptr<const Scope>>>;

        static auto ResolveSpecialSymbol(
            const SymbolResolutionContext& context
        ) -> std::optional<Expected<ISymbol*>>;

        Compilation* m_Compilation{};
        size_t m_NestLevel{};
        std::optional<std::string> m_OptName{};
        std::optional<std::string> m_OptAnonymousName{};
        std::optional<std::shared_ptr<Scope>> m_OptParent{};
        std::vector<std::weak_ptr<Scope>> m_Children{};
        std::unordered_map<std::string, std::vector<std::unique_ptr<ISymbol>>> m_SymbolMap;
        GenericInstantiator m_GenericInstantiator;
    };
}
