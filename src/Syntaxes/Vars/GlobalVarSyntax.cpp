#include "Syntaxes/Vars/GlobalVarSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Vars/GlobalVarSymbol.hpp"

namespace Ace
{
    GlobalVarSyntax::GlobalVarSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
        const AccessModifier accessModifier
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_AccessModifier{ accessModifier }
    {
    }

    auto GlobalVarSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto GlobalVarSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto GlobalVarSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Attributes).Build();
    }

    auto GlobalVarSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto GlobalVarSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto GlobalVarSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
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
            std::make_unique<GlobalVarSymbol>(
                GetSymbolScope(),
                m_AccessModifier,
                m_Name,
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }
}
