#include "Symbols/FunctionSymbol.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Assert.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/Aliases/ImplSelfAliasTypeSymbol.hpp"
#include "Noun.hpp"
#include "TypeInfo.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"
#include "Emittable.hpp"

namespace Ace
{
    FunctionSymbol::FunctionSymbol(
        const std::shared_ptr<Scope>& bodyScope,
        const SymbolCategory category,
        const AccessModifier accessModifier,
        const Ident& name,
        ITypeSymbol* const type,
        const std::vector<ITypeSymbol*>& typeArgs
    ) : m_BodyScope{ bodyScope },
        m_Category{ category },
        m_AccessModifier{ accessModifier },
        m_Name{ name },
        m_Type{ type },
        m_TypeArgs{ typeArgs }
    {
    }

    auto FunctionSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "function" };
    }

    auto FunctionSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto FunctionSymbol::GetCategory() const -> SymbolCategory
    {
        return m_Category;
    }

    auto FunctionSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto FunctionSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto FunctionSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<FunctionSymbol>(
            scope->CreateChild(),
            GetCategory(),
            GetAccessModifier(),
            GetName(),
            CreateInstantiated<ITypeSymbol>(GetType(), context),
            context.TypeArgs
        );
    }

    auto FunctionSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }

    auto FunctionSymbol::SetBodyScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        m_BodyScope = scope;
    }

    auto FunctionSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        return m_TypeArgs;
    }

    auto FunctionSymbol::GetConstrainedScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto FunctionSymbol::BindBlockSema(
        const std::shared_ptr<const BlockStmtSema>& blockSema
    ) -> void
    {
        ACE_ASSERT(!m_OptBlockSema.has_value());
        m_OptBlockSema = blockSema;
        m_OptEmittableBlock = blockSema;
    }

    auto FunctionSymbol::GetBlockSema() -> const std::optional<std::shared_ptr<const BlockStmtSema>>&
    {
        return m_OptBlockSema;
    }

    auto FunctionSymbol::BindEmittableBlock(
        const std::shared_ptr<const IEmittable<void>>& emittableBlock
    ) -> void
    {
        ACE_ASSERT(!m_OptEmittableBlock.has_value());
        m_OptEmittableBlock = emittableBlock;
    }

    auto FunctionSymbol::GetEmittableBlock() const -> const std::optional<std::shared_ptr<const IEmittable<void>>>&
    {
        return m_OptEmittableBlock;
    }
}
