#include "Syntaxes/Vars/Params/NormalParamVarSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"

namespace Ace
{
    NormalParamVarSyntax::NormalParamVarSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
        const size_t index
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_Index{ index }
    {
    }

    auto NormalParamVarSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalParamVarSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalParamVarSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Attributes).Build();
    }

    auto NormalParamVarSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto NormalParamVarSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto NormalParamVarSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
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
            std::make_unique<NormalParamVarSymbol>(
                GetSymbolScope(),
                m_Name,
                typeSymbol,
                m_Index
            ),
            std::move(diagnostics),
        };
    }
}
