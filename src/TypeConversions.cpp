#include "TypeConversions.hpp"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"
#include "TypeInfo.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression/ConversionPlaceholder.hpp"

namespace Ace
{
    static auto GetNativeConversionOperator(
        Symbol::Type::IBase* t_fromType, 
        Symbol::Type::IBase* t_toType, 
        const std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>& t_fromOperatorMap
    ) -> std::optional<Symbol::Function*>
    {
        const auto foundTypeIt = t_fromOperatorMap.find(t_toType);
        if (foundTypeIt == end(t_fromOperatorMap))
        {
            return std::nullopt;
        }

        const auto foundOperatorIt = foundTypeIt->second.find(t_fromType);
        if (foundOperatorIt == end(foundTypeIt->second))
        {
            return std::nullopt;
        }

        return foundOperatorIt->second;
    }

    auto GetImplicitConversionOperator(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Type::IBase* t_fromType,
        Symbol::Type::IBase* t_toType
    ) -> Expected<Symbol::Function*>
    {
        const auto optNativeOperator = GetNativeConversionOperator(
            t_fromType,
            t_toType,
            t_scope->GetCompilation()->Natives->GetImplicitFromOperatorMap()
        );
        if (optNativeOperator.has_value())
        {
            return optNativeOperator.value();
        }

        auto name = t_toType->CreateFullyQualifiedName();
        name.Sections.emplace_back(
            std::string{ SpecialIdentifier::Operator::ImplicitFrom }
        );

        ACE_TRY(operatorSymbol, t_scope->ResolveStaticSymbol<Symbol::Function>(name));
        return operatorSymbol;
    }

    auto GetExplicitConversionOperator(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Type::IBase* t_fromType,
        Symbol::Type::IBase* t_toType
    ) -> Expected<Symbol::Function*>
    {
        const auto optNativeImplicitOperator = GetNativeConversionOperator(
            t_fromType,
            t_toType,
            t_scope->GetCompilation()->Natives->GetImplicitFromOperatorMap()
        );
        if (optNativeImplicitOperator.has_value())
        {
            return optNativeImplicitOperator.value();
        }

        const auto optNativeExplicitOperator = GetNativeConversionOperator(
            t_fromType,
            t_toType,
            t_scope->GetCompilation()->Natives->GetExplicitFromOperatorMap()
        );
        if (optNativeExplicitOperator)
        {
            return optNativeExplicitOperator.value();
        }

        auto name = t_toType->CreateFullyQualifiedName();
        name.Sections.emplace_back(std::string{});

        name.Sections.back().Name = SpecialIdentifier::Operator::ExplicitFrom;
        const auto expExplicitOperatorSymbol =
            t_scope->ResolveStaticSymbol<Symbol::Function>(name);
        if (expExplicitOperatorSymbol)
        {
            return expExplicitOperatorSymbol.Unwrap();
        }

        name.Sections.back().Name = SpecialIdentifier::Operator::ImplicitFrom;
        const auto expImplicitOperatorSymbol =
            t_scope->ResolveStaticSymbol<Symbol::Function>(name);
        if (expImplicitOperatorSymbol)
        {
            return expImplicitOperatorSymbol.Unwrap();
        }

        ACE_TRY_UNREACHABLE();
    }

    auto AreTypesSame(
        const std::vector<Symbol::Type::IBase*>& t_typesA,
        const std::vector<Symbol::Type::IBase*>& t_typesB
    ) -> bool
    {
        if (t_typesA.size() != t_typesB.size())
        {
            return false;
        }

        for (size_t i = 0; i < t_typesA.size(); i++)
        {
            if (
                t_typesA.at(i)->GetUnaliased() !=
                t_typesB.at(i)->GetUnaliased()
                )
            {
                return false;
            }
        }

        return true;
    }

    auto AreTypesConvertible(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<TypeInfo>& t_fromTypeInfos,
        const std::vector<TypeInfo>& t_targetTypeInfos
    ) -> bool
    {
        if (t_fromTypeInfos.size() != t_targetTypeInfos.size())
        {
            return false;
        }

        for (size_t i = 0; i < t_fromTypeInfos.size(); i++)
        {
            const auto dummyExpression = std::make_shared<const BoundNode::Expression::ConversionPlaceholder>(
                t_scope,
                t_fromTypeInfos.at(i)
            );

            const auto expConvertedExpression = BoundNode::Expression::CreateImplicitlyConverted(
                dummyExpression,
                t_targetTypeInfos.at(i)
            );
            if (expConvertedExpression)
            {
                continue;
            }

            return false;
        }

        return true;
    }
}
