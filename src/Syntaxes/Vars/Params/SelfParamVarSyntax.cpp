#include "Syntaxes/Vars/Params/SelfParamVarSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "SpecialIdent.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Ident.hpp"

namespace Ace
{
    SelfParamVarSyntax::SelfParamVarSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeName& typeName
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ srcLocation, SpecialIdent::Self },
        m_TypeName{ typeName }
    {
    }

    auto SelfParamVarSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SelfParamVarSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto SelfParamVarSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto SelfParamVarSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto SelfParamVarSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ISizedTypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<SelfParamVarSymbol>(
                GetSrcLocation(),
                GetSymbolScope(),
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }
}
