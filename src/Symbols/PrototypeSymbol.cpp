#include "Symbols/PrototypeSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Symbols/Types/TraitSelfTypeSymbol.hpp"

namespace Ace
{
    PrototypeSymbol::PrototypeSymbol(
        const std::shared_ptr<Scope>& bodyScope,
        const SymbolCategory category,
        const Ident& name,
        const size_t index,
        TraitTypeSymbol* const parentTrait,
        ITypeSymbol* const type,
        ITypeSymbol* const selfType,
        const std::vector<ITypeSymbol*>& typeArgs
    ) : m_BodyScope{ bodyScope },
        m_Category{ category },
        m_Name{ name },
        m_Index{ index },
        m_ParentTrait{ parentTrait },
        m_Type{ type },
        m_SelfType{ selfType },
        m_TypeArgs{ typeArgs }
    {
        ACE_ASSERT(
            GetScope() ==
            dynamic_cast<TraitTypeSymbol*>(
                m_ParentTrait->GetRoot()
            )->GetPrototypeScope()
        );
    }

    auto PrototypeSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "prototype" };
    }

    auto PrototypeSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto PrototypeSymbol::GetCategory() const -> SymbolCategory
    {
        return m_Category;
    }

    auto PrototypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Pub;
    }

    auto PrototypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto PrototypeSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<PrototypeSymbol>(
            scope->CreateChild(),
            GetCategory(),
            GetName(),
            GetIndex(),
            CreateInstantiated<TraitTypeSymbol>(m_ParentTrait, context),
            CreateInstantiated<ITypeSymbol>(GetType(), context),
            CreateInstantiated<ITypeSymbol>(m_SelfType, context),
            context.TypeArgs
        );
    }

    auto PrototypeSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }

    auto PrototypeSymbol::SetBodyScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        m_BodyScope = scope;
    }

    auto PrototypeSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        return m_TypeArgs;
    }

    auto PrototypeSymbol::GetConstrainedScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto PrototypeSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }

    auto PrototypeSymbol::GetParentTrait() const -> TraitTypeSymbol*
    {
        return m_ParentTrait;
    }

    static auto IsParamDynSafe(
        const PrototypeSymbol* const prototype,
        IParamVarSymbol* const param
    ) -> bool
    {
        return
            param->GetType()->GetUnaliased() !=
            prototype->CollectSelfType().value()->GetUnaliased();
    }

    auto PrototypeSymbol::IsDynDispatchable() const -> bool
    {
        const auto optSelfParam = CollectSelfParam();
        if (
            !optSelfParam.has_value() ||
            !optSelfParam.value()->GetType()->GetWithoutRef()->IsDynStrongPtr()
            )
        {
            return false;
        }

        if (!GetTypeArgs().empty())
        {
            return false;
        }

        if (
            GetType()->GetUnaliased() ==
            CollectSelfType().value()->GetUnaliased()
            )
        {
            return false;
        }

        const auto params = CollectParams();
        const auto unsafeParamIt = std::find_if_not(begin(params), end(params),
        [&](IParamVarSymbol* const param)
        {
            return IsParamDynSafe(this, param);
        });
        if (unsafeParamIt != end(params))
        {
            return false;
        }

        return true;
    }

    auto PrototypeSymbol::GetSelfType() const -> ITypeSymbol*
    {
        return m_SelfType;
    }
}
