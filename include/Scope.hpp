#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <iterator>
#include <set>
#include <unordered_map>
#include <tuple>

#include "Diagnostic.hpp"
#include "Diagnostics/SymbolTypeDiagnostics.hpp"
#include "Name.hpp"
#include "SymbolCategory.hpp"
#include "Ident.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    class Scope;

    class Compilation;
    class ISymbolCreatable;

    class ISymbol;
    class ModuleSymbol;
    class ITypeSymbol;
    class ITemplateSymbol;
    class ITemplatableSymbol;

    auto CreateNameSectionSrcLocation(
        const SymbolNameSection& nameSection
    ) -> SrcLocation;

    auto DiagnoseSymbolNotVisible(
        const SrcLocation& srcLocation,
        ISymbol* const symbol,
        const std::shared_ptr<const Scope>& beginScope
    ) -> Diagnosed<void>;

    template<typename TSymbol>
    auto IsCorrectSymbolType(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol
    ) -> Expected<void>
    {
        DiagnosticBag diagnosticBag{};

        if (dynamic_cast<const TSymbol*>(symbol) == nullptr)
        {
            return diagnosticBag.Add(CreateIncorrectSymbolTypeError<TSymbol>(
                srcLocation
            ));
        }

        return Void{ diagnosticBag };
    }

    auto IsCorrectSymbolCategory(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol,
        const SymbolCategory symbolCategory
    ) -> Expected<void>;

    auto CreateSymbolRedefinitionError(
        const ISymbol* const redefinedSymbol
    ) -> std::shared_ptr<const Diagnostic>;

    template<typename TSymbol>
    constexpr auto IsTemplate() -> bool
    {
        return std::is_base_of_v<ITemplateSymbol, TSymbol>;
    }

    auto CastToTemplatableSymbol(
        const ISymbol* const symbol
    ) -> const ITemplatableSymbol*;
    auto CollectTemplateArgs(
        const ITemplatableSymbol* const templatableSymbol
    ) -> std::vector<ITypeSymbol*>;
    auto CollectImplTemplateArgs(
        const ITemplatableSymbol* const templatableSymbol
    ) -> std::vector<ITypeSymbol*>;

    struct SymbolResolutionData
    {
        SymbolResolutionData(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const Scope>& beginScope,
            const std::vector<SymbolNameSection>::const_iterator nameSectionsBegin,
            const std::vector<SymbolNameSection>::const_iterator nameSectionsEnd,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
            const std::function<Expected<void>(const Ace::SrcLocation&, const ISymbol*)>& isCorrectSymbolType,
            const std::vector<std::shared_ptr<const Scope>>& scopes,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const bool isTemplate
        ) : SymbolResolutionData
            {
                srcLocation,
                beginScope,
                nameSectionsBegin,
                nameSectionsEnd,
                optArgTypes,
                isCorrectSymbolType,
                scopes,
                implTemplateArgs,
                {},
                isTemplate
            }
        {
        }
        SymbolResolutionData(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const Scope>& beginScope,
            const std::vector<SymbolNameSection>::const_iterator nameSectionsBegin,
            const std::vector<SymbolNameSection>::const_iterator nameSectionsEnd,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
            const std::function<Expected<void>(const Ace::SrcLocation&, const ISymbol*)>& isCorrectSymbolType,
            const std::vector<std::shared_ptr<const Scope>>& scopes,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs,
            const bool isTemplate
        ) : SrcLocation{ srcLocation },
            BeginScope{ beginScope },
            NameSectionsBegin{ nameSectionsBegin },
            NameSectionsEnd{ nameSectionsEnd },
            OptArgTypes{ optArgTypes },
            IsCorrectSymbolType{ isCorrectSymbolType },
            Scopes{ scopes },
            ImplTemplateArgs{ implTemplateArgs },
            TemplateArgs{ templateArgs },
            IsTemplate{ isTemplate },
            IsLastNameSection
            {
                std::distance(nameSectionsBegin, nameSectionsEnd) == 1
            },
            Name{ nameSectionsBegin->Name.String },
            TemplateName{ SpecialIdent::CreateTemplate(Name) }
        {
        }

        SrcLocation SrcLocation{};
        std::shared_ptr<const Scope> BeginScope{};
        std::vector<SymbolNameSection>::const_iterator NameSectionsBegin{};
        std::vector<SymbolNameSection>::const_iterator NameSectionsEnd{};
        std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>> OptArgTypes{};
        std::function<Expected<void>(const Ace::SrcLocation&, const ISymbol*)> IsCorrectSymbolType{};
        std::vector<std::shared_ptr<const Scope>> Scopes{};
        std::vector<ITypeSymbol*> ImplTemplateArgs{};
        std::vector<ITypeSymbol*> TemplateArgs{};
        bool IsTemplate{};
        bool IsLastNameSection{};
        std::string Name{};
        std::string TemplateName{};
    };

    class GlobalScope
    {
    public:
        GlobalScope();
        GlobalScope(const Compilation* const compilation);
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
        
        auto GetCompilation() const -> const Compilation*;
        auto GetNestLevel() const -> size_t;
        auto GetParent() const -> const std::optional<std::shared_ptr<Scope>>&;
        auto GetName() const -> const std::string&;
        auto FindModule() const -> std::optional<ModuleSymbol*>;

        auto GetOrCreateChild(
            const std::optional<std::string>& optName
        ) -> std::shared_ptr<Scope>;
        auto HasChild(
            const std::shared_ptr<const Scope>& scope
        ) const -> bool;

        template<typename TSymbol>
        static auto DefineSymbol(
            std::unique_ptr<TSymbol> ownedSymbol
        ) -> Diagnosed<TSymbol*>
        {
            DiagnosticBag diagnosticBag{};

            // TODO: Dont allow private types to leak in public interface
            /*
            This doesnt work for associated functions params, needs rework:

            if (auto typedSymbol = dynamic_cast<const ISymbol*>(symbol))
            {
                const bool isTypePrivate =
                    typedSymbol->GetType()->GetAccessModifier() ==
                    AccessModifier::Private;

                const bool isSymbolPrivate =
                    typedSymbol->GetAccessModifier() ==
                    AccessModifier::Private;

                if (isTypePrivate && !isSymbolPrivate)
                {
                    Leak of private type in public interface
                }
            }
            */

            auto* const symbol = ownedSymbol.get();
            const auto scope = symbol->GetScope();

            std::vector<ITypeSymbol*> templateArgs{};
            std::vector<ITypeSymbol*> implTemplateArgs{};

            const auto* const templatableSymbol =
                CastToTemplatableSymbol(symbol);
            if (templatableSymbol)
            {
                templateArgs = Ace::CollectTemplateArgs(templatableSymbol);
                implTemplateArgs = Ace::CollectImplTemplateArgs(
                    templatableSymbol
                );
            }

            const auto optSameSymbol = scope->GetDefinedSymbol(
                symbol->GetName().String, 
                templateArgs, 
                implTemplateArgs
            );
            if (optSameSymbol.has_value())
            {
                diagnosticBag.Add(CreateSymbolRedefinitionError(symbol));

                auto* const sameSymbol = optSameSymbol.value();

                const bool isSameKind = 
                    sameSymbol->GetKind() == symbol->GetKind();

                auto* const castedSameSymbol = dynamic_cast<TSymbol*>(
                    sameSymbol
                );
                const bool isSameType = castedSameSymbol != nullptr;

                if (isSameKind && isSameType)
                {
                    return Diagnosed{ castedSameSymbol, diagnosticBag };
                }
            }

            scope->m_SymbolMap[symbol->GetName().String].push_back(
                std::move(ownedSymbol)
            );

            return Diagnosed{ symbol, diagnosticBag };
        }
        static auto DefineSymbol(
            const ISymbolCreatable* const creatable
        ) -> Diagnosed<ISymbol*>;
        static auto RemoveSymbol(
            ISymbol* const symbol
        ) -> void;

        auto DefineAssociation(
            const std::shared_ptr<Scope>& association
        ) -> void;

        static auto CreateArgTypes(
            ITypeSymbol* const argType
        ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>;
        static auto CreateArgTypes(
            const std::vector<ITypeSymbol*>& argTypes
        ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>;

        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const Ident& name
        ) const -> Expected<TSymbol*>
        {
            return ResolveStaticSymbol<TSymbol>(name, {});
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const Ident& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<TSymbol*>
        {
            const SymbolName symbolName
            {
                SymbolNameSection{ name },
                SymbolNameResolutionScope::Local,
            };

            return ResolveStaticSymbol<TSymbol>(
                symbolName,
                optArgTypes
            );
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const SymbolName& name
        ) const -> Expected<TSymbol*>
        {
            return ResolveStaticSymbol<TSymbol>(name, {});
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const SymbolName& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<TSymbol*>
        {
            DiagnosticBag diagnosticBag{};

            const auto srcLocation = CreateNameSectionSrcLocation(
                name.Sections.front()
            );

            const auto expBeginScope = GetStaticSymbolResolutionBeginScope(
                srcLocation,
                name
            );
            diagnosticBag.Add(expBeginScope);
            if (!expBeginScope)
            {
                return diagnosticBag;
            }

            std::vector<std::shared_ptr<const Scope>> beginScopes{};
            beginScopes.push_back(expBeginScope.Unwrap());
            beginScopes.insert(
                end(beginScopes), 
                begin(expBeginScope.Unwrap()->m_Associations), 
                end  (expBeginScope.Unwrap()->m_Associations)
            );

            const auto expSymbol = ResolveSymbolInScopes(SymbolResolutionData{
                srcLocation,
                shared_from_this(),
                name.Sections.begin(),
                name.Sections.end(),
                optArgTypes,
                IsCorrectSymbolType<TSymbol>,
                beginScopes,
                GetStaticSymbolResolutionImplTemplateArgs(shared_from_this()),
                IsTemplate<TSymbol>()
            });
            diagnosticBag.Add(expSymbol);
            if (!expSymbol)
            {
                return diagnosticBag;
            }

            auto* const symbol = expSymbol.Unwrap();

            const auto expIsCorrectSymbolCategory = IsCorrectSymbolCategory(
                srcLocation,
                symbol,
                SymbolCategory::Static
            );
            diagnosticBag.Add(expIsCorrectSymbolCategory);
            if (!expIsCorrectSymbolCategory)
            {
                return diagnosticBag;
            }

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_ASSERT(castedSymbol);

            return Expected{ castedSymbol, diagnosticBag };
        }

        template<typename TSymbol>
        auto ResolveInstanceSymbol(
            ITypeSymbol* const selfType,
            const SymbolNameSection& name
        ) const -> Expected<TSymbol*>
        {
            return ResolveInstanceSymbol<TSymbol>(
                selfType,
                name,
                {}
            );
        }
        template<typename TSymbol>
        auto ResolveInstanceSymbol(
            ITypeSymbol* const selfType,
            const SymbolNameSection& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<TSymbol*>
        {
            DiagnosticBag diagnosticBag{};

            const std::vector nameSections{ name };

            const auto srcLocation = CreateNameSectionSrcLocation(
                nameSections.front()
            );

            const auto expSymbol = ResolveSymbolInScopes(SymbolResolutionData{
                srcLocation,
                shared_from_this(),
                nameSections.begin(),
                nameSections.end(),
                optArgTypes,
                IsCorrectSymbolType<TSymbol>,
                GetInstanceSymbolResolutionScopes(selfType),
                CollectInstanceSymbolResolutionImplTemplateArgs(selfType),
                IsTemplate<TSymbol>()
            });
            diagnosticBag.Add(expSymbol);
            if (!expSymbol)
            {
                return diagnosticBag;
            }

            auto* const symbol = expSymbol.Unwrap();

            const auto expIsCorrectSymbolCategory = IsCorrectSymbolCategory(
                srcLocation,
                symbol,
                SymbolCategory::Instance
            );
            diagnosticBag.Add(expIsCorrectSymbolCategory);
            if (!expIsCorrectSymbolCategory)
            {
                return diagnosticBag;
            }

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_ASSERT(castedSymbol);

            return Expected{ castedSymbol, diagnosticBag };
        }

        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const Ident& name
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(name, {});
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const Ident& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(
                name,
                optArgTypes,
                {},
                {}
            );
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const Ident& name,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(
                name,
                {},
                implTemplateArgs,
                templateArgs
            );
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const Ident& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs
        ) const -> Expected<TSymbol*>
        {
            DiagnosticBag diagnosticBag{};

            const std::vector nameSections
            { 
                SymbolNameSection{ name }
            };

            const auto srcLocation = CreateNameSectionSrcLocation(
                nameSections.front()
            );

            const auto expSymbol = ResolveSymbolInScopes(SymbolResolutionData{
                srcLocation,
                shared_from_this(),
                nameSections.begin(),
                nameSections.end(),
                optArgTypes,
                IsCorrectSymbolType<TSymbol>,
                std::vector{ shared_from_this() },
                implTemplateArgs,
                templateArgs,
                IsTemplate<TSymbol>()
            });
            diagnosticBag.Add(expSymbol);
            if (!expSymbol)
            {
                return diagnosticBag;
            }

            auto* const symbol = expSymbol.Unwrap();

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_ASSERT(castedSymbol);

            return Expected{ castedSymbol, diagnosticBag };
        }

        template<typename TSymbol>
        auto CollectSymbols() const -> std::vector<TSymbol*>
        {
            std::vector<TSymbol*> symbols{};

            std::for_each(begin(m_SymbolMap), end(m_SymbolMap),
            [&](const std::pair<const std::string&, const std::vector<std::unique_ptr<ISymbol>>&>& pair)
            {
                std::for_each(begin(pair.second), end(pair.second),
                [&](const std::unique_ptr<ISymbol>& ownedSymbol)
                {
                    auto* const symbol = dynamic_cast<TSymbol*>(
                        ownedSymbol.get()
                    );
                    if (!symbol)
                    {
                        return;
                    }

                    symbols.push_back(symbol);
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
                const auto childSymbols = child.lock()->CollectSymbolsRecursive<TSymbol>();
                symbols.insert(
                    end(symbols), 
                    begin(childSymbols), 
                    end  (childSymbols)
                );
            });

            return symbols;
        }

        auto CollectAllDefinedSymbols() const -> std::vector<ISymbol*>;
        auto CollectAllDefinedSymbolsRecursive() const -> std::vector<ISymbol*>;

        static auto ResolveOrInstantiateTemplateInstance(
            const SrcLocation& srcLocation,
            ITemplateSymbol* const t3mplate,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs
        ) -> Expected<ISymbol*>;

        auto CollectTemplateArgs() const -> std::vector<ITypeSymbol*>;
        auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>;

        auto DefineTemplateArgAliases(
            const std::vector<Ident>& implTemplateParamNames,
            const std::vector<ITypeSymbol*> implTemplateArgs,
            const std::vector<Ident>& templateParamNames,
            const std::vector<ITypeSymbol*> templateArgs
        ) -> Diagnosed<void>;

    private:
        Scope(const Compilation* const compilation);
        auto Clear() -> void;

        Scope(
            const Compilation* const compilation,
            const std::optional<std::string>& optName,
            const std::optional<std::shared_ptr<Scope>>& optParent
        );

        auto AddChild(const std::optional<std::string>& optName) -> std::shared_ptr<Scope>;

        auto GetDefinedSymbol(
            const std::string& name,
            const std::vector<ITypeSymbol*>& templateArgs,
            const std::vector<ITypeSymbol*>& implTemplateArgs
        ) -> std::optional<ISymbol*>;

        static auto ResolveSymbolInScopes(
            SymbolResolutionData data
        ) -> Expected<ISymbol*>;

        static auto CollectMatchingSymbolsInScopes(
            const SymbolResolutionData& data
        ) -> Diagnosed<std::vector<ISymbol*>>;

        auto CollectMatchingSymbols(
            const SymbolResolutionData& data
        ) const -> Expected<std::vector<ISymbol*>>;
        auto CollectMatchingNormalSymbols(
            const SymbolResolutionData& data,
            const std::vector<std::unique_ptr<ISymbol>>& matchingNameSymbols
        ) const -> Expected<std::vector<ISymbol*>>;
        auto CollectMatchingTemplateSymbols(
            const SymbolResolutionData& data,
            const std::vector<std::unique_ptr<ISymbol>>& matchingNameSymbols
        ) const -> Expected<std::vector<ISymbol*>>;

        static auto ResolveTemplateInstance(
            const SrcLocation& srcLocation,
            const ITemplateSymbol* const t3mplate,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs
        ) -> Expected<ISymbol*>;

        static auto GetInstanceSymbolResolutionScopes(
            ITypeSymbol* selfType
        ) -> std::vector<std::shared_ptr<const Scope>>;
        static auto CollectInstanceSymbolResolutionImplTemplateArgs(
            ITypeSymbol* const selfType
        ) -> std::vector<ITypeSymbol*>;

        auto GetStaticSymbolResolutionBeginScope(
            const SrcLocation& srcLocation,
            const SymbolName& name
        ) const -> Expected<std::shared_ptr<const Scope>>;
        static auto GetStaticSymbolResolutionImplTemplateArgs(
            const std::shared_ptr<const Scope>& beginScope
        ) -> std::vector<ITypeSymbol*>;

        static auto ResolveNameSectionSymbol(
            const SymbolResolutionData& data,
            const std::vector<ISymbol*>& matchingSymbols
        ) -> Expected<ISymbol*>;

        auto CollectSelfAndAssociations() const -> std::vector<std::shared_ptr<const Scope>>;

        const Compilation* m_Compilation{};
        size_t m_NestLevel{};
        std::string m_Name{};
        std::optional<std::shared_ptr<Scope>> m_OptParent{};
        std::vector<std::weak_ptr<Scope>> m_Children{};
        std::set<std::shared_ptr<Scope>> m_Associations{};
        std::unordered_map<std::string, std::vector<std::unique_ptr<ISymbol>>> m_SymbolMap;
    };
}
