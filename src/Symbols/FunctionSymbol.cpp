#include "Symbols/FunctionSymbol.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <algorithm>
#include <iterator>

#include "Assert.hpp"
#include "Scope.hpp"
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
#include "Utility.hpp"

namespace Ace
{
    FunctionSymbol::FunctionSymbol(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::string& t_name,
        const SymbolCategory t_symbolCategory,
        const AccessModifier t_accessModifier,
        ITypeSymbol* const t_type
    ) : m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_SymbolCategory{ t_symbolCategory },
        m_AccessModifier{ t_accessModifier },
        m_Type{ t_type }
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

    auto FunctionSymbol::GetName() const -> const std::string&
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
            const NormalParamVarSymbol* const t_lhs, 
            const NormalParamVarSymbol* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
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
            [&](IParamVarSymbol* const t_param)
            {
                auto* const normalParam =
                    dynamic_cast<NormalParamVarSymbol*>(t_param);

                if (normalParam)
                {
                    normalParams.push_back(normalParam);
                    return;
                }

                auto* const selfParam =
                    dynamic_cast<SelfParamVarSymbol*>(t_param);

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
            const NormalParamVarSymbol* const t_lhs,
            const NormalParamVarSymbol* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
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
            [](const IParamVarSymbol* const t_param)
            {
                return TypeInfo{ t_param->GetType(), ValueKind::R };
            }
        );

        return typeInfos;
    }

    auto FunctionSymbol::BindBody(const std::shared_ptr<const IEmittable<void>>& t_body) -> void
    {
        m_OptBody = t_body;
    }

    auto FunctionSymbol::GetBody() -> std::optional<const IEmittable<void>*>
    {
        if (!m_OptBody.has_value())
            return {};

        return m_OptBody.value().get();
    }

    auto FunctionSymbol::GetTemplate() const -> std::optional<FunctionTemplateSymbol*>
    {
        auto expTemplate = GetScope()->ResolveStaticSymbol<FunctionTemplateSymbol>(
            SpecialIdentifier::CreateTemplate(m_Name)
        );

        return expTemplate ? expTemplate.Unwrap() : std::optional<FunctionTemplateSymbol*>{};
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
