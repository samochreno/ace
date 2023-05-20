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
    auto Function::CollectParameters() const -> std::vector<Symbol::Variable::Parameter::IBase*>
    {
        auto normalParameters = m_SelfScope->CollectDefinedSymbols<Symbol::Variable::Parameter::Normal>();
        std::sort(begin(normalParameters), end(normalParameters),
        [](
            const Symbol::Variable::Parameter::Normal* const t_lhs, 
            const Symbol::Variable::Parameter::Normal* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        std::vector<Symbol::Variable::Parameter::IBase*> parameters{};
        parameters.insert(
            end(parameters),
            begin(normalParameters),
            end  (normalParameters)
        );

        return parameters;
    }

    auto Function::CollectAllParameters() const -> std::vector<Symbol::Variable::Parameter::IBase*>
    {
        const auto definedParameters = m_SelfScope->CollectDefinedSymbols<Symbol::Variable::Parameter::IBase>();

        std::vector<Symbol::Variable::Parameter::Self*> selfParameters{};
        std::vector<Symbol::Variable::Parameter::Normal*> normalParameters{};
        std::for_each(
            begin(definedParameters),
            end  (definedParameters),
            [&](Symbol::Variable::Parameter::IBase* const t_parameter)
            {
                auto* const normalParameter =
                    dynamic_cast<Symbol::Variable::Parameter::Normal*>(t_parameter);

                if (normalParameter)
                {
                    normalParameters.push_back(normalParameter);
                    return;
                }

                auto* const selfParameter =
                    dynamic_cast<Symbol::Variable::Parameter::Self*>(t_parameter);

                if (selfParameter)
                {
                    selfParameters.push_back(selfParameter);
                    return;
                }
            }
        );

        std::vector<Symbol::Variable::Parameter::IBase*> parameters{};

        if (!selfParameters.empty())
        {
            ACE_ASSERT(selfParameters.size() == 1);
            parameters.push_back(selfParameters.front());
        }

        std::sort(begin(normalParameters), end(normalParameters),
        [](
            const Symbol::Variable::Parameter::Normal* const t_lhs,
            const Symbol::Variable::Parameter::Normal* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });
        parameters.insert(
            end(parameters),
            begin(normalParameters), 
            end  (normalParameters)
        );

        return parameters;
    }

    auto Function::CollectArgumentTypeInfos() const -> std::vector<TypeInfo>
    {
        const auto parameters = CollectParameters();

        std::vector<TypeInfo> typeInfos{};
        std::transform(
            begin(parameters),
            end  (parameters),
            back_inserter(typeInfos),
            [](const Symbol::Variable::Parameter::IBase* const t_parameter)
            {
                return TypeInfo{ t_parameter->GetType(), ValueKind::R };
            }
        );

        return typeInfos;
    }

    auto Function::BindBody(const std::shared_ptr<const IEmittable<void>>& t_body) -> void
    {
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
