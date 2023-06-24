#include "Symbol/Function.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <algorithm>
#include <iterator>

#include "Asserts.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Symbol/Var/Param/Base.hpp"
#include "Symbol/Var/Param/Self.hpp"
#include "Symbol/Var/Param/Normal.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Template/Function.hpp"
#include "Symbol/Type/Alias/TemplateArg/Normal.hpp"
#include "Symbol/Type/Alias/TemplateArg/Impl.hpp"
#include "Emittable.hpp"
#include "TypeInfo.hpp"
#include "Utility.hpp"

namespace Ace::Symbol
{
    Function::Function(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::string& t_name,
        const SymbolCategory& t_symbolCategory,
        const AccessModifier& t_accessModifier,
        Symbol::Type::IBase* const t_type
    ) : m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_SymbolCategory{ t_symbolCategory },
        m_AccessModifier{ t_accessModifier },
        m_Type{ t_type }
    {
    }

    auto Function::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Function::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto Function::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Function::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Function;
    }

    auto Function::GetSymbolCategory() const -> SymbolCategory
    {
        return m_SymbolCategory;
    }

    auto Function::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto Function::GetType() const -> Symbol::Type::IBase*
    {
        return m_Type;
    }

    auto Function::CollectParams() const -> std::vector<Symbol::Var::Param::IBase*>
    {
        auto normalParams = m_SelfScope->CollectSymbols<Symbol::Var::Param::Normal>();
        std::sort(begin(normalParams), end(normalParams),
        [](
            const Symbol::Var::Param::Normal* const t_lhs, 
            const Symbol::Var::Param::Normal* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        std::vector<Symbol::Var::Param::IBase*> params{};
        params.insert(
            end(params),
            begin(normalParams),
            end  (normalParams)
        );

        return params;
    }

    auto Function::CollectAllParams() const -> std::vector<Symbol::Var::Param::IBase*>
    {
        const auto definedParams = m_SelfScope->CollectSymbols<Symbol::Var::Param::IBase>();

        std::vector<Symbol::Var::Param::Self*> selfParams{};
        std::vector<Symbol::Var::Param::Normal*> normalParams{};
        std::for_each(
            begin(definedParams),
            end  (definedParams),
            [&](Symbol::Var::Param::IBase* const t_param)
            {
                auto* const normalParam =
                    dynamic_cast<Symbol::Var::Param::Normal*>(t_param);

                if (normalParam)
                {
                    normalParams.push_back(normalParam);
                    return;
                }

                auto* const selfParam =
                    dynamic_cast<Symbol::Var::Param::Self*>(t_param);

                if (selfParam)
                {
                    selfParams.push_back(selfParam);
                    return;
                }
            }
        );

        std::vector<Symbol::Var::Param::IBase*> params{};

        if (!selfParams.empty())
        {
            ACE_ASSERT(selfParams.size() == 1);
            params.push_back(selfParams.front());
        }

        std::sort(begin(normalParams), end(normalParams),
        [](
            const Symbol::Var::Param::Normal* const t_lhs,
            const Symbol::Var::Param::Normal* const t_rhs
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

    auto Function::CollectArgTypeInfos() const -> std::vector<TypeInfo>
    {
        const auto params = CollectParams();

        std::vector<TypeInfo> typeInfos{};
        std::transform(
            begin(params),
            end  (params),
            back_inserter(typeInfos),
            [](const Symbol::Var::Param::IBase* const t_param)
            {
                return TypeInfo{ t_param->GetType(), ValueKind::R };
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
    
    auto Function::CollectTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_SelfScope->CollectTemplateArgs();
    }

    auto Function::CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        return m_SelfScope->CollectImplTemplateArgs();
    }
}
