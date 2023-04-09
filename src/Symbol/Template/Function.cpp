#include "Symbol/Template/Function.hpp"

#include <vector>
#include <string>
#include <unordered_set>

#include "Asserts.hpp"
#include "Error.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Normal.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Impl.hpp"
#include "Symbol/Function.hpp"
#include "Core.hpp"
#include "Name.hpp"

namespace Ace::Symbol::Template
{
    auto Function::InstantiateSymbols(
        const std::vector<Symbol::Type::IBase*>& t_implArguments,
        const std::vector<Symbol::Type::IBase*>& t_arguments
    ) -> Symbol::IBase*
    {
        const auto& implParameters = m_TemplateNode->GetImplParameters();
        const auto& parameters = m_TemplateNode->GetParameters();

        ACE_ASSERT(t_implArguments.size() == implParameters.size());
        ACE_ASSERT(t_arguments.size() == parameters.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScope(m_TemplateNode->GetScope());

        ast->GetSelfScope()->DefineTemplateArgumentAliases(
            implParameters,
            t_implArguments,
            parameters,
            t_arguments
        );

        const auto nodes = Core::GetAllNodes(ast);

        Core::CreateAndDefineSymbols(nodes).Unwrap();
        Core::DefineAssociations(nodes).Unwrap();

        m_InstantiatedOnlySymbolsASTs.push_back(ast);

        return Scope::ResolveOrInstantiateTemplateInstance(
            this,
            t_implArguments,
            t_arguments
        ).Unwrap();
    }

    auto Function::InstantiateSemanticsForSymbols() -> void
    {
        std::for_each(begin(m_InstantiatedOnlySymbolsASTs), end(m_InstantiatedOnlySymbolsASTs), []
        (const std::shared_ptr<const Node::Function>& t_ast)
        {
            const auto boundAST = Core::CreateBoundAndVerifiedAST(
                t_ast,
                [](const std::shared_ptr<const Node::Function>& t_ast) { return t_ast->CreateBound(); },
                [](const std::shared_ptr<const BoundNode::Function>& t_ast) { return t_ast->GetOrCreateTypeChecked({}); },
                [](const std::shared_ptr<const BoundNode::Function>& t_ast) { return t_ast->GetOrCreateLowered({}); },
                [](const std::shared_ptr<const BoundNode::Function>& t_ast) { return t_ast->GetOrCreateTypeChecked({}); }
            ).Unwrap();

            Core::SetFunctionSymbolsBodies(Core::GetAllNodes(boundAST));
        });

        m_InstantiatedOnlySymbolsASTs.clear();
    }
}
