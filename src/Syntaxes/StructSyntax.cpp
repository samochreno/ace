#include "Syntaxes/StructSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/Vars/FieldVarSyntax.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    StructSyntax::StructSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        const AccessModifier accessModifier,
        const Ident& name,
        const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
        const std::vector<std::shared_ptr<const FieldVarSyntax>>& fields,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope },
        m_AccessModifier{ accessModifier },
        m_Name{ name },
        m_Attributes{ attributes },
        m_Fields{ fields },
        m_TypeParams{ typeParams }
    {
    }

    auto StructSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StructSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto StructSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Attributes)
            .Collect(m_Fields)
            .Collect(m_TypeParams)
            .Build();
    }

    auto StructSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto StructSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::Type;
    }

    auto StructSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<StructTypeSymbol>(
                m_BodyScope,
                m_AccessModifier,
                m_Name,
                ResolveTypeParamSymbols(m_BodyScope, m_TypeParams)
            ),
            DiagnosticBag::Create(),
        };
    }
}
