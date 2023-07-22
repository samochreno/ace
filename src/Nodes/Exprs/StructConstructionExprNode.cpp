#include "Nodes/Exprs/StructConstructionExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Nodes/Exprs/SymbolLiteralExprNode.hpp"
#include "Name.hpp"

namespace Ace
{
    StructConstructionExprNode::StructConstructionExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& typeName,
        std::vector<StructConstructionExprArg>&& args
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeName{ typeName },
        m_Args{ args }
    {
    }

    auto StructConstructionExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StructConstructionExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StructConstructionExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        std::for_each(begin(m_Args), end(m_Args),
        [&](const StructConstructionExprArg& arg)
        {
            if (arg.OptValue.has_value())
            {
                AddChildren(children, arg.OptValue.value());
            }
        });

        return children;
    }

    auto StructConstructionExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const StructConstructionExprNode>
    {
        std::vector<StructConstructionExprArg> clonedArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(clonedArgs),
        [&](const StructConstructionExprArg& arg)
        {
            const auto clonedOptValue = [&]() -> std::optional<std::shared_ptr<const IExprNode>>
            {
                if (!arg.OptValue.has_value())
                {
                    return std::nullopt;
                }

                return arg.OptValue.value()->CloneInScopeExpr(scope);
            }();

            return StructConstructionExprArg
            {
                arg.Name,
                clonedOptValue,
            };
        });

        return std::make_shared<const StructConstructionExprNode>(
            m_SrcLocation,
            scope,
            m_TypeName,
            std::move(clonedArgs)
        );
    }

    auto StructConstructionExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto StructConstructionExprNode::CreateBound() const -> Expected<std::shared_ptr<const StructConstructionExprBoundNode>>
    {
        ACE_TRY(structSymbol, m_Scope->ResolveStaticSymbol<StructTypeSymbol>(m_TypeName));

        const auto varSymbols = structSymbol->GetVars();
        ACE_TRY_ASSERT(varSymbols.size() == m_Args.size());

        ACE_TRY(boundArgs, TransformExpectedVector(m_Args,
        [&](const StructConstructionExprArg& arg) -> Expected<StructConstructionExprBoundArg>
        {
            const auto matchingVarSymbolIt = std::find_if(
                begin(varSymbols),
                end  (varSymbols),
                [&](const InstanceVarSymbol* const varSymbol)
                {
                    return
                        varSymbol->GetName().String ==
                        arg.Name.String;
                }
            );

            ACE_TRY_ASSERT(matchingVarSymbolIt != end(varSymbols));
            auto* const varSymbol = *matchingVarSymbolIt;

            ACE_TRY(boundValue, ([&]() -> Expected<std::shared_ptr<const IExprBoundNode>>
            {
                if (arg.OptValue.has_value())
                {
                    return arg.OptValue.value()->CreateBoundExpr();
                }

                const SymbolName symbolName
                {
                    SymbolNameSection{ arg.Name },
                    SymbolNameResolutionScope::Local,
                };

                return std::make_shared<const SymbolLiteralExprNode>(
                    arg.Name.SrcLocation,
                    m_Scope,
                    symbolName
                )->CreateBoundExpr();
            }()));

            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(varSymbol, m_Scope));

            return StructConstructionExprBoundArg
            {
                varSymbol,
                boundValue,
            };
        }));

        return std::make_shared<const StructConstructionExprBoundNode>(
            GetSrcLocation(),
            GetScope(),
            structSymbol,
            boundArgs
        );
    }

    auto StructConstructionExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
