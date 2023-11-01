#include "Syntaxes/Impls/InherentImplSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/ConstraintSyntax.hpp"
#include "Syntaxes/FunctionSyntax.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Impls/InherentImplSymbol.hpp"

namespace Ace
{
    InherentImplSyntax::InherentImplSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
        const SymbolName& typeName,
        const std::vector<std::shared_ptr<const ConstraintSyntax>>& constraints,
        const std::vector<std::shared_ptr<const FunctionSyntax>>& functions
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope },
        m_TypeParams{ typeParams },
        m_TypeName{ typeName },
        m_Constraints{ constraints },
        m_Functions{ functions }
    {
    }

    auto InherentImplSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto InherentImplSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto InherentImplSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_TypeParams)
            .Collect(m_Constraints)
            .Collect(m_Functions)
            .Build();
    }

    auto InherentImplSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto InherentImplSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto InherentImplSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTypeSymbol = diagnostics.Collect(
            m_BodyScope->ResolveStaticSymbol<INominalTypeSymbol>(m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetNominalType()
        );

        return Diagnosed
        {
            std::make_unique<InherentImplSymbol>(
                SrcLocation{ GetSrcLocation(), m_TypeName.CreateSrcLocation() },
                m_BodyScope,
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }
}
