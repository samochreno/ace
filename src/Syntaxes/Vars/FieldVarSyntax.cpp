#include "Syntaxes/Vars/FieldVarSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Vars/FieldVarSymbol.hpp"

namespace Ace
{
    FieldVarSyntax::FieldVarSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const AccessModifier accessModifier,
        const SymbolName& parentStructName,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
        const size_t index
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_AccessModifier{ accessModifier },
        m_ParentStructName{ parentStructName },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_Index{ index }
    {
    }

    auto FieldVarSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FieldVarSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto FieldVarSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Attributes).Build();
    }

    auto FieldVarSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto FieldVarSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto FieldVarSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const parentStructSymbol = DiagnosticBag::CreateNoError().Collect(
            GetScope()->ResolveStaticSymbol<StructTypeSymbol>(m_ParentStructName)
        ).value();

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ISizedTypeSymbol>(GetScope(), m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<FieldVarSymbol>(
                parentStructSymbol,
                m_AccessModifier,
                m_Name,
                typeSymbol,
                m_Index
            ),
            std::move(diagnostics),
        };
    }
}
