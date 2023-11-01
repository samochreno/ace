#include "Syntaxes/PrototypeSyntax.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Name.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/Vars/Params/SelfParamVarSyntax.hpp"
#include "Syntaxes/Vars/Params/NormalParamVarSyntax.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/ConstraintSyntax.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/PrototypeSymbol.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    PrototypeSyntax::PrototypeSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        const SymbolName& parentTraitName,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
        const size_t index,
        const std::optional<std::shared_ptr<const SelfParamVarSyntax>>& optSelfParam,
        const std::vector<std::shared_ptr<const NormalParamVarSyntax>>& params,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
        const std::vector<std::shared_ptr<const ConstraintSyntax>>& constraints
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope },
        m_ParentTraitName{ parentTraitName },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_Index{ index },
        m_OptSelfParam{ optSelfParam },
        m_Params{ params },
        m_TypeParams{ typeParams },
        m_Constraints{ constraints }
    {
    }

    auto PrototypeSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto PrototypeSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto PrototypeSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Attributes)
            .Collect(m_OptSelfParam)
            .Collect(m_Params)
            .Collect(m_TypeParams)
            .Collect(m_Constraints)
            .Build();
    }

    auto PrototypeSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto PrototypeSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto PrototypeSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto symbolCategory = m_OptSelfParam.has_value() ?
            SymbolCategory::Instance :
            SymbolCategory::Static;

        auto* const parentTraitSymbol = DiagnosticBag::CreateNoError().Collect(
            m_BodyScope->ResolveStaticSymbol<TraitTypeSymbol>(m_ParentTraitName)
        ).value();

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ITypeSymbol>(m_BodyScope, m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        auto* const selfTypeSymbol = DiagnosticBag::CreateNoError().Collect(
            m_BodyScope->ResolveSelfType(SrcLocation{ GetCompilation() })
        ).value();

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<PrototypeSymbol>(
                m_BodyScope,
                symbolCategory,
                m_Name,
                m_Index,
                parentTraitSymbol,
                typeSymbol,
                selfTypeSymbol,
                ResolveTypeParamSymbols(m_BodyScope, m_TypeParams)
            ),
            std::move(diagnostics),
        };
    }
}
