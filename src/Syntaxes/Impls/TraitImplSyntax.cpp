#include "Syntaxes/Impls/TraitImplSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/ConstraintSyntax.hpp"
#include "Syntaxes/FunctionSyntax.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Impls/TraitImplSymbol.hpp"

namespace Ace
{
    TraitImplSyntax::TraitImplSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
        const SymbolName& traitName,
        const SymbolName& typeName,
        const std::vector<std::shared_ptr<const ConstraintSyntax>>& constraints,
        const std::shared_ptr<const ImplSelfSyntax>& self,
        const std::vector<std::shared_ptr<const FunctionSyntax>>& functions
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope },
        m_TypeParams{ typeParams },
        m_TraitName{ traitName },
        m_TypeName{ typeName },
        m_Constraints{ constraints },
        m_Self{ self },
        m_Functions{ functions }
    {
    }

    auto TraitImplSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto TraitImplSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto TraitImplSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_TypeParams)
            .Collect(m_Constraints)
            .Collect(m_Self)
            .Collect(m_Functions)
            .Build();
    }

    auto TraitImplSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto TraitImplSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto TraitImplSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTraitSymbol = diagnostics.Collect(
            m_BodyScope->ResolveStaticSymbol<TraitTypeSymbol>(m_TraitName)
        );
        auto* const traitSymbol = optTraitSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetTrait()
        );

        const auto optTypeSymbol = diagnostics.Collect(
            m_BodyScope->ResolveStaticSymbol<ITypeSymbol>(m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return Diagnosed
        {
            std::make_unique<TraitImplSymbol>(
                SrcLocation{ GetSrcLocation(), m_TypeName.CreateSrcLocation() },
                m_BodyScope,
                traitSymbol,
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }
}
