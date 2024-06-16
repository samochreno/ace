#include "PlaceholderOverlapping.hpp"

#include <vector>

#include "Symbols/ConstraintSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/Types/TypeParamTypeSymbol.hpp"

namespace Ace
{
    auto DoPlaceholdersOverlap(
        ITypeSymbol* conversative,
        ITypeSymbol* speculative
    ) -> bool
    {
        conversative = conversative->GetUnaliasedType();
         speculative =  speculative->GetUnaliasedType();

         if (
             conversative->CreateSignature() == "std::Add[std::?658::?659_add::Other]" &&
             speculative->CreateSignature() == "std::Add[std::Int]"
            )
         {
             [](){}();
         }

        if (conversative == speculative)
        {
            return true;
        }

        auto* const conversativeParam =
            dynamic_cast<TypeParamTypeSymbol*>(conversative);
        auto* const speculativeParam =
            dynamic_cast<TypeParamTypeSymbol*>(speculative);

        if (conversativeParam && speculativeParam)
        {
            return true;
        }

        if (speculativeParam)
        {
            const auto scope = speculativeParam->GetScope();
            const auto traits =
                scope->CollectConstrainedTraits(speculativeParam);

            const auto unimplementedTrait = std::find_if_not(
                begin(traits),
                end  (traits),
                [&](TraitTypeSymbol* const trait)
                {
                    return Scope::CollectImplOfFor(
                        trait,
                        conversative
                    ).has_value();
                }
            );
            return unimplementedTrait == end(traits);
        }

        if (conversative->GetRoot() != speculative->GetRoot())
        {
            return false;
        }

        const auto& conversativeArgs = conversative->GetTypeArgs();
        const auto&  speculativeArgs =  speculative->GetTypeArgs();
        ACE_ASSERT(conversativeArgs.size() == speculativeArgs.size());

        for (size_t i = 0; i < conversativeArgs.size(); i++)
        {
            const auto doOverlap = DoPlaceholdersOverlap(
                conversativeArgs.at(i),
                speculativeArgs.at(i)
            );
            if (!doOverlap)
            {
                return false;
            }
        }

        return true;
    }
}
