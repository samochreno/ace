#include "Symbols/FunctionSymbol.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <utility>
#include <algorithm>
#include <iterator>

#include "Assert.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
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
        const Ident& name,
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

    auto FunctionSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto FunctionSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::Function;
    }

    auto FunctionSymbol::GetCategory() const -> SymbolCategory
    {
        return m_SymbolCategory;
    }

    auto FunctionSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
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

    auto FunctionSymbol::CollectSelfParam() const -> std::optional<SelfParamVarSymbol*>
    {
        const auto selfParams =
            m_SelfScope->CollectSymbols<SelfParamVarSymbol>();

        if (selfParams.empty())
        {
            return {};
        }
        
        ACE_ASSERT(selfParams.size() == 1);
        return selfParams.front();
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

    auto FunctionSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
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
        const Ident name
        {
            m_Name.SrcLocation,
            SpecialIdent::CreateTemplate(m_Name.String)
        };

        return DiagnosticBag::Create().Collect(
            GetScope()->ResolveStaticSymbol<FunctionTemplateSymbol>(name)
        );
    }
}
