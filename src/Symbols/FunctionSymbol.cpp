#include "Symbols/FunctionSymbol.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <utility>
#include <algorithm>
#include <iterator>

#include "Assert.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/ImplTemplateArgAliasTypeSymbol.hpp"
#include "Emittable.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    FunctionSymbol::FunctionSymbol(
        const std::shared_ptr<Scope>& selfScope,
        const Identifier& name,
        const SymbolCategory symbolCategory,
        const AccessModifier accessModifier,
        ITypeSymbol* const type
    ) : m_SelfScope{ selfScope },
        m_Name{ name },
        m_SymbolCategory{ symbolCategory },
        m_AccessModifier{ accessModifier },
        m_Type{ type }
    {
    }

    auto FunctionSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto FunctionSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto FunctionSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto FunctionSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Function;
    }

    auto FunctionSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return m_SymbolCategory;
    }

    auto FunctionSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto FunctionSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }

    auto FunctionSymbol::CollectParams() const -> std::vector<IParamVarSymbol*>
    {
        auto normalParams = m_SelfScope->CollectSymbols<NormalParamVarSymbol>();
        std::sort(begin(normalParams), end(normalParams),
        [](
            const NormalParamVarSymbol* const lhs, 
            const NormalParamVarSymbol* const rhs
            )
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });

        std::vector<IParamVarSymbol*> params{};
        params.insert(
            end(params),
            begin(normalParams),
            end  (normalParams)
        );

        return params;
    }

    auto FunctionSymbol::CollectAllParams() const -> std::vector<IParamVarSymbol*>
    {
        const auto definedParams =
            m_SelfScope->CollectSymbols<IParamVarSymbol>();

        std::vector<SelfParamVarSymbol*> selfParams{};
        std::vector<NormalParamVarSymbol*> normalParams{};
        std::for_each(
            begin(definedParams),
            end  (definedParams),
            [&](IParamVarSymbol* const param)
            {
                auto* const normalParam =
                    dynamic_cast<NormalParamVarSymbol*>(param);

                if (normalParam)
                {
                    normalParams.push_back(normalParam);
                    return;
                }

                auto* const selfParam =
                    dynamic_cast<SelfParamVarSymbol*>(param);

                if (selfParam)
                {
                    selfParams.push_back(selfParam);
                    return;
                }
            }
        );

        std::vector<IParamVarSymbol*> params{};

        if (!selfParams.empty())
        {
            ACE_ASSERT(selfParams.size() == 1);
            params.push_back(selfParams.front());
        }

        std::sort(begin(normalParams), end(normalParams),
        [](
            const NormalParamVarSymbol* const lhs,
            const NormalParamVarSymbol* const rhs
            )
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });
        params.insert(
            end(params),
            begin(normalParams), 
            end  (normalParams)
        );

        return params;
    }

    auto FunctionSymbol::CollectArgTypeInfos() const -> std::vector<TypeInfo>
    {
        const auto params = CollectParams();

        std::vector<TypeInfo> typeInfos{};
        std::transform(
            begin(params),
            end  (params),
            back_inserter(typeInfos),
            [](const IParamVarSymbol* const param)
            {
                return TypeInfo{ param->GetType(), ValueKind::R };
            }
        );

        return typeInfos;
    }

    auto FunctionSymbol::BindBody(const std::shared_ptr<const IEmittable<void>>& body) -> void
    {
        m_OptBody = body;
    }

    auto FunctionSymbol::GetBody() -> std::optional<const IEmittable<void>*>
    {
        if (!m_OptBody.has_value())
        {
            return {};
        }

        return m_OptBody.value().get();
    }

    auto FunctionSymbol::GetTemplate() const -> std::optional<FunctionTemplateSymbol*>
    {
        const Identifier name
        {
            m_Name.SourceLocation,
            SpecialIdentifier::CreateTemplate(m_Name.String)
        };

        auto expTemplate =
            GetScope()->ResolveStaticSymbol<FunctionTemplateSymbol>(name);

        return expTemplate ?
            expTemplate.Unwrap() :
            std::optional<FunctionTemplateSymbol*>{};
    }
    
    auto FunctionSymbol::CollectTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return m_SelfScope->CollectTemplateArgs();
    }

    auto FunctionSymbol::CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        return m_SelfScope->CollectImplTemplateArgs();
    }
}
