#include "Symbol/Template/Type.hpp"

#include <vector>
#include <string>

#include "Asserts.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Normal.hpp"
#include "Core.hpp"

namespace Ace::Symbol::Template
{
    auto Type::InstantiateSymbols(
        const std::vector<Symbol::Type::IBase*>& t_implArguments,
        const std::vector<Symbol::Type::IBase*>& t_arguments
    ) -> Symbol::IBase*
    {
        const auto& parameters = m_TemplateNode->GetParameters();

        ACE_ASSERT(t_arguments.size() == parameters.size());

        const auto ast = m_TemplateNode->GetAST()->CloneInScopeType(m_TemplateNode->GetScope());

        ast->GetSelfScope()->DefineTemplateArgumentAliases(
            {},
            {},
            parameters,
            t_arguments
        );

        const auto nodes = Core::GetAllNodes(ast);

        Core::CreateAndDefineSymbols(GetCompilation(), nodes).Unwrap();
        Core::DefineAssociations(GetCompilation(), nodes).Unwrap();

        m_InstantiatedOnlySymbolsASTs.push_back(ast);

        return Scope::ResolveOrInstantiateTemplateInstance(
            this,
            t_implArguments,
            t_arguments
        ).Unwrap();
    }

    auto Type::InstantiateSemanticsForSymbols() -> void
    {
        std::for_each(begin(m_InstantiatedOnlySymbolsASTs), end(m_InstantiatedOnlySymbolsASTs),
        [&](const std::shared_ptr<const Node::Type::IBase>& t_ast)
        {
            const auto boundAST = Core::CreateBoundTransformedAndVerifiedAST(
                GetCompilation(),
                t_ast,
                [](const std::shared_ptr<const Node::Type::IBase>& t_ast) { return t_ast->CreateBoundType(); },
                [](const std::shared_ptr<const BoundNode::Type::IBase>& t_ast) { return t_ast->GetOrCreateTypeCheckedType({}); },
                [](const std::shared_ptr<const BoundNode::Type::IBase>& t_ast) { return t_ast->GetOrCreateLoweredType({}); },
                [](const std::shared_ptr<const BoundNode::Type::IBase>& t_ast) { return t_ast->GetOrCreateTypeCheckedType({}); }
            ).Unwrap();

            Core::BindFunctionSymbolsBodies(
                GetCompilation(),
                Core::GetAllNodes(boundAST)
            );

            auto* const selfSymbol = boundAST->GetSymbol();

            auto copyOperatorName = selfSymbol->CreateFullyQualifiedName();
            copyOperatorName.Sections.push_back(SymbolNameSection{ SpecialIdentifier::Operator::Copy });
            selfSymbol->GetScope()->ResolveStaticSymbol<Symbol::Function>(copyOperatorName);

            auto dropOperatorName = selfSymbol->CreateFullyQualifiedName();
            dropOperatorName.Sections.push_back(SymbolNameSection{ SpecialIdentifier::Operator::Drop });
            selfSymbol->GetScope()->ResolveStaticSymbol<Symbol::Function>(dropOperatorName);
        });

        m_InstantiatedOnlySymbolsASTs.clear();
    }
}
