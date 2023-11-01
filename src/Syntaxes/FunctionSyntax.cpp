#include "Syntaxes/FunctionSyntax.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Name.hpp"
#include "AccessModifier.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/ImplSelfSyntax.hpp"
#include "Syntaxes/Vars/Params/SelfParamVarSyntax.hpp"
#include "Syntaxes/Vars/Params/NormalParamVarSyntax.hpp"
#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/ConstraintSyntax.hpp"
#include "Diagnostic.hpp"
#include "TypeResolution.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    FunctionSyntax::FunctionSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        const AccessModifier accessModifier,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
        const std::optional<std::shared_ptr<const ImplSelfSyntax>>& optSelf,
        const std::optional<std::shared_ptr<const SelfParamVarSyntax>>& optSelfParam,
        const std::vector<std::shared_ptr<const NormalParamVarSyntax>>& params,
        const std::optional<std::shared_ptr<const BlockStmtSyntax>>& optBlock,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
        const std::vector<std::shared_ptr<const ConstraintSyntax>>& constraints
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope },
        m_AccessModifier{ accessModifier },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_OptSelf{ optSelf },
        m_OptSelfParam{ optSelfParam },
        m_Params{ params },
        m_OptBlock{ optBlock },
        m_TypeParams{ typeParams },
        m_Constraints{ constraints }
    {
    }

    auto FunctionSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FunctionSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto FunctionSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_Attributes)
            .Collect(m_OptSelf)
            .Collect(m_OptSelfParam)
            .Collect(m_Params)
            .Collect(m_OptBlock)
            .Collect(m_TypeParams)
            .Collect(m_Constraints)
            .Build();
    }

    auto FunctionSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto FunctionSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto FunctionSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto symbolCategory = m_OptSelfParam.has_value() ?
            SymbolCategory::Instance :
            SymbolCategory::Static;

        const auto optTypeSymbol = diagnostics.Collect(
            ResolveTypeSymbol<ITypeSymbol>(m_BodyScope, m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<FunctionSymbol>(
                m_BodyScope,
                symbolCategory,
                m_AccessModifier,
                m_Name,
                typeSymbol,
                ResolveTypeParamSymbols(m_BodyScope, m_TypeParams)
            ),
            std::move(diagnostics),
        };
    }

    auto FunctionSyntax::GetBlock() const -> const std::optional<std::shared_ptr<const BlockStmtSyntax>>&
    {
        return m_OptBlock;
    }
}
