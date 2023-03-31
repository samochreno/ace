#include "Symbol/Function.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <algorithm>
#include <iterator>

#include "Asserts.hpp"
#include "Symbol/Variable/Parameter/Base.hpp"
#include "Symbol/Variable/Parameter/Self.hpp"
#include "Symbol/Variable/Parameter/Normal.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Template/Function.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Normal.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Impl.hpp"
#include "Emittable.hpp"
#include "TypeInfo.hpp"
#include "Scope.hpp"
#include "Utility.hpp"

namespace Ace::Symbol
{
    auto Function::GetAllParameters() const -> std::vector<Symbol::Variable::Parameter::IBase*>
    {
        std::vector<Symbol::Variable::Parameter::IBase*> parameters{};

        const auto definedParameters = m_SelfScope->CollectDefinedSymbols<Symbol::Variable::Parameter::IBase>();

        const auto selfParameters = DynamicCastFilter<Symbol::Variable::Parameter::Self*>(definedParameters);
        if (selfParameters.size() > 0)
        {
            ACE_ASSERT(selfParameters.size() == 1);
            parameters.push_back(selfParameters.front());
        }

        auto normalParameters = DynamicCastFilter<Symbol::Variable::Parameter::Normal*>(definedParameters);
        std::sort(begin(normalParameters), end(normalParameters), []
        (
            const Symbol::Variable::Parameter::Normal* const t_lhs,
            const Symbol::Variable::Parameter::Normal* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        parameters.insert(end(parameters), begin(normalParameters), end(normalParameters));
        return parameters;
    }

    auto Function::GetParameters() const -> std::vector<Symbol::Variable::Parameter::Normal*>
    {
        auto normalParameters = m_SelfScope->CollectDefinedSymbols<Symbol::Variable::Parameter::Normal>();

        std::sort(begin(normalParameters), end(normalParameters), []
        (
            const Symbol::Variable::Parameter::Normal* const t_lhs, 
            const Symbol::Variable::Parameter::Normal* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        return normalParameters;
    }

    auto Function::GetArgumentTypeInfos() const -> std::vector<TypeInfo>
    {
        auto parameters = GetParameters();

        std::vector<TypeInfo> typeInfos{};
        std::transform(begin(parameters), end(parameters), back_inserter(typeInfos), []
        (const Symbol::Variable::Parameter::Normal* const t_parameter)
        {
            return TypeInfo{ t_parameter->GetType(), ValueKind::R };
        });

        return typeInfos;
    }

    auto Function::SetBody(const std::shared_ptr<const IEmittable<void>>& t_body) -> void
    {
        ACE_ASSERT(!m_OptBody.has_value());
        m_OptBody = t_body;
    }

    auto Function::GetBody() -> std::optional<const IEmittable<void>*>
    {
        if (!m_OptBody.has_value())
            return {};

        return m_OptBody.value().get();
    }

    auto Function::GetTemplate() const -> std::optional<Symbol::Template::Function*>
    {
        auto expTemplate = GetScope()->ResolveStaticSymbol<Symbol::Template::Function>(
            SpecialIdentifier::CreateTemplate(m_Name)
            );

        return expTemplate ? expTemplate.Unwrap() : std::optional<Symbol::Template::Function*>{};
    }
    
    auto Function::CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_SelfScope->CollectTemplateArguments();
    }

    auto Function::CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_SelfScope->CollectImplTemplateArguments();
    }
}
