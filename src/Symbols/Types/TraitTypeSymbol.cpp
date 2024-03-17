#include "Symbols/Types/TraitTypeSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Symbols/SupertraitSymbol.hpp"
#include "Symbols/Types/TraitSelfTypeSymbol.hpp"
#include "Noun.hpp"
#include "Symbols/PrototypeSymbol.hpp"

namespace Ace
{
    TraitTypeSymbol::TraitTypeSymbol(
        const std::shared_ptr<Scope>& bodyScope,
        const std::shared_ptr<Scope>& prototypeScope,
        const AccessModifier accessModifier,
        const Ident& name,
        const std::vector<ITypeSymbol*>& typeArgs
    ) : m_BodyScope{ bodyScope },
        m_PrototypeScope{ prototypeScope },
        m_AccessModifier{ accessModifier },
        m_Name{ name },
        m_TypeArgs{ typeArgs }
    {
    }

    auto TraitTypeSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "trait" };
    }

    auto TraitTypeSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto TraitTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto TraitTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto TraitTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto TraitTypeSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<TraitTypeSymbol>(
            scope->CreateChild(),
            scope->CreateChild(),
            GetAccessModifier(),
            GetName(),
            context.TypeArgs
        );
    }

    auto TraitTypeSymbol::SetBodyScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        m_BodyScope = scope;
    }

    auto TraitTypeSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        return m_TypeArgs;
    }

    auto TraitTypeSymbol::GetPrototypeScope() const -> const std::shared_ptr<Scope>&
    {
        return m_PrototypeScope;
    }

    auto TraitTypeSymbol::CollectPrototypes() const -> std::vector<PrototypeSymbol*>
    {
        auto prototypes =
            GetPrototypeScope()->CollectSymbols<PrototypeSymbol>();

        std::set<PrototypeSymbol*> prototypeSet{};
        std::for_each(begin(prototypes), end(prototypes),
        [&](PrototypeSymbol* const prototype)
        {
            prototypeSet.insert(
                dynamic_cast<PrototypeSymbol*>(prototype->GetRoot())
            );
        });

        prototypes = std::vector<PrototypeSymbol*>{
            begin(prototypeSet),
            end  (prototypeSet)
        };

        std::sort(begin(prototypes), end(prototypes),
        [](
            PrototypeSymbol* const lhsPrototype,
            PrototypeSymbol* const rhsPrototype
            )
        {
            return lhsPrototype->GetIndex() < rhsPrototype->GetIndex();
        });


        return prototypes;
    }

    auto TraitTypeSymbol::CollectSelf() const -> TraitSelfTypeSymbol*
    {
        return GetBodyScope()->CollectSymbols<TraitSelfTypeSymbol>().front();
    }

    auto TraitTypeSymbol::CollectSupertraits() const -> std::vector<SupertraitSymbol*>
    {
        const auto allSupertraits =
            GetBodyScope()->CollectSymbols<SupertraitSymbol>();

        std::vector<SupertraitSymbol*> supertraits{};
        std::copy_if(
            begin(allSupertraits),
            end  (allSupertraits),
            back_inserter(supertraits),
            [](SupertraitSymbol* const supertrait)
            {
                return !supertrait->GetTrait()->IsError();
            }
        );

        return supertraits;
    }
}
