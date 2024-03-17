#include "Syntaxes/TraitSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/PrototypeSyntax.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/TypeReimportSyntax.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    TraitSyntax::TraitSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        const std::shared_ptr<Scope>& prototypeScope,
        const AccessModifier accessModifier,
        const Ident& name,
        const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
        const std::shared_ptr<const TraitSelfSyntax>& self,
        const std::vector<std::shared_ptr<const PrototypeSyntax>>& prototypes,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
        const std::vector<std::shared_ptr<const TypeReimportSyntax>>& typeParamReimports,
        const std::vector<std::shared_ptr<const SupertraitSyntax>>& supertraits
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope },
        m_PrototypeScope{ prototypeScope },
        m_AccessModifier{ accessModifier },
        m_Name{ name },
        m_Attributes{ attributes },
        m_Self{ self },
        m_Prototypes{ prototypes },
        m_TypeParams{ typeParams },
        m_TypeParamReimports{ typeParamReimports },
        m_Supertraits{ supertraits }
    {
    }

    auto TraitSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto TraitSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto TraitSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Attributes)
            .Collect(m_Self)
            .Collect(m_Prototypes)
            .Collect(m_TypeParams)
            .Collect(m_TypeParamReimports)
            .Collect(m_Supertraits)
            .Build();
    }

    auto TraitSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto TraitSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::Type;
    }

    auto TraitSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<TraitTypeSymbol>(
                m_BodyScope,
                m_PrototypeScope,
                m_AccessModifier,
                m_Name,
                ResolveTypeParamSymbols(m_BodyScope, m_TypeParams)
            ),
            DiagnosticBag::Create(),
        };
    }
}
