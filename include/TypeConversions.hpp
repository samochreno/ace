#pragma once

#include <memory>
#include <vector>

#include "Symbol/Type/Base.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    auto GetImplicitConversionOperator(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Type::IBase* t_fromType,
        Symbol::Type::IBase* t_targetType
    ) -> Expected<Symbol::Function*>;
    auto GetExplicitConversionOperator(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Type::IBase* t_fromType,
        Symbol::Type::IBase* t_targetType
    ) -> Expected<Symbol::Function*>;

    auto AreTypesSame(
        const std::vector<Symbol::Type::IBase*>& t_typesA,
        const std::vector<Symbol::Type::IBase*>& t_typesB
    ) -> bool;
    auto AreTypesConvertible(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<TypeInfo>& t_fromTypeInfos,
        const std::vector<TypeInfo>& t_targetTypeInfos
    ) -> bool;
}
