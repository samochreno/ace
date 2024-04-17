#include "Symbols/CallableSymbol.hpp"

#include <vector>
#include <optional>

#include "Assert.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    auto ICallableSymbol::CollectParams() const -> std::vector<IParamVarSymbol*>
    {
        auto params = GetBodyScope()->CollectSymbols<NormalParamVarSymbol>();
        std::sort(begin(params), end(params),
        [](
            const NormalParamVarSymbol* const lhs, 
            const NormalParamVarSymbol* const rhs
            )
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });

        return { begin(params), end(params) };
    }

    auto ICallableSymbol::CollectAllParams() const -> std::vector<IParamVarSymbol*>
    {
        const auto declaredParams =
            GetBodyScope()->CollectSymbols<IParamVarSymbol>();

        std::vector<SelfParamVarSymbol*> selfParams{};
        std::vector<NormalParamVarSymbol*> normalParams{};
        std::for_each(begin(declaredParams), end(declaredParams),
        [&](IParamVarSymbol* const param)
        {
            auto* const normalParam =
                dynamic_cast<NormalParamVarSymbol*>(param);
            if (normalParam)
            {
                normalParams.push_back(normalParam);
                return;
            }

            auto* const selfParam = dynamic_cast<SelfParamVarSymbol*>(param);
            if (selfParam)
            {
                selfParams.push_back(selfParam);
                return;
            }
        });

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
        params.insert(end(params), begin(normalParams), end(normalParams));

        return params;
    }

    auto ICallableSymbol::CollectSelfParam() const -> std::optional<SelfParamVarSymbol*>
    {
        const auto selfParams =
            GetBodyScope()->CollectSymbols<SelfParamVarSymbol>();

        if (selfParams.empty())
        {
            return {};
        }
        
        ACE_ASSERT(selfParams.size() == 1);
        return selfParams.front();
    }

    auto ICallableSymbol::CollectParamTypes() const -> std::vector<ITypeSymbol*>
    {
        const auto params = CollectParams();

        std::vector<ITypeSymbol*> types{};
        std::transform(
            begin(params),
            end  (params),
            back_inserter(types),
            [](IParamVarSymbol* const param) { return param->GetType(); }
        );
        
        return types;
    }

    auto ICallableSymbol::CollectArgTypeInfos() const -> std::vector<TypeInfo>
    {
        const auto params = CollectParams();

        std::vector<TypeInfo> typeInfos{};
        std::transform(begin(params), end(params), back_inserter(typeInfos),
        [](const IParamVarSymbol* const param)
        {
            return TypeInfo{ param->GetType(), ValueKind::R };
        });

        return typeInfos;
    }

    auto ICallableSymbol::CollectAllArgTypeInfos() const -> std::vector<TypeInfo>
    {
        const auto params = CollectAllParams();

        std::vector<TypeInfo> typeInfos{};
        std::transform(begin(params), end(params), back_inserter(typeInfos),
        [](const IParamVarSymbol* const param)
        {
            return TypeInfo{ param->GetType(), ValueKind::R };
        });

        return typeInfos;
    }

    auto ICallableSymbol::CollectSelfType() const -> std::optional<ITypeSymbol*>
    {
        return DiagnosticBag::Create().Collect(
            GetBodyScope()->ResolveSelfType(SrcLocation{ GetCompilation() })
        );
    }
}
