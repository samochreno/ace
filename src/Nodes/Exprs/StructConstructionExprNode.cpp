#include "Nodes/Exprs/StructConstructionExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
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

    auto StructConstructionExprNode::CollectChildren() const -> std::vector<const INode*>
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

    static auto CreateBoundArgValue(
        const std::shared_ptr<Scope>& scope,
        const StructConstructionExprArg& arg
    ) -> std::shared_ptr<const IExprBoundNode>
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
            scope,
            symbolName
        )->CreateBoundExpr();
    }

    static auto CreateVarSymbolToUseSrcLocationsMap(
        const std::vector<InstanceVarSymbol*>& varSymbols
    ) -> std::map<InstanceVarSymbol*, std::vector<SrcLocation>>
    {
        std::map<InstanceVarSymbol*, std::vector<SrcLocation>> map{};

        std::for_each(begin(varSymbols), end(varSymbols),
        [&](InstanceVarSymbol* const varSymbol)
        {
            map[varSymbol] = std::vector<SrcLocation>{};
        });

        return map;
    }

    static auto DiagnoseInaccessibleVar(
        const std::shared_ptr<Scope>& scope,
        InstanceVarSymbol* const varSymbol,
        const std::vector<SrcLocation>& varUseSrcLocations
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        std::for_each(begin(varUseSrcLocations), end(varUseSrcLocations),
        [&](const SrcLocation& varUseSrcLocation)
        {
            diagnostics.Add(DiagnoseInaccessibleSymbol(
                varUseSrcLocation,
                varSymbol,
                scope
            ));
        });

        return Diagnosed<void>{ diagnostics };
    }

    static auto DiagnoseInaccessibleVars(
        const std::shared_ptr<Scope>& scope,
        const std::vector<InstanceVarSymbol*>& varSymbols,
        const std::map<InstanceVarSymbol*, std::vector<SrcLocation>>& varSymbolToUseSrcLocationsMap
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        std::for_each(begin(varSymbols), end(varSymbols),
        [&](InstanceVarSymbol* const varSymbol)
        {
            diagnostics.Add(DiagnoseInaccessibleVar(
                scope,
                varSymbol,
                varSymbolToUseSrcLocationsMap.at(varSymbol)
            ));
        });

        return Diagnosed<void>{ diagnostics };
    }

    static auto DiagnoseMissingVars(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        const std::vector<InstanceVarSymbol*>& varSymbols,
        const std::map<InstanceVarSymbol*, std::vector<SrcLocation>>& varSymbolToUseSrcLocationsMap
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        std::vector<InstanceVarSymbol*> missingVarSymbols{};
        std::for_each(begin(varSymbols), end(varSymbols),
        [&](InstanceVarSymbol* const varSymbol)
        {
            if (varSymbolToUseSrcLocationsMap.at(varSymbol).empty())
            {
                missingVarSymbols.push_back(varSymbol);
            }
        });

        if (!missingVarSymbols.empty())
        {
            diagnostics.Add(CreateMissingStructVarsError(
                srcLocation,
                structSymbol,
                missingVarSymbols
            ));
        }

        return Diagnosed<void>{ diagnostics };
    }

    static auto DiagnoseStructConstructionVarSpecifiedMoreThanOnce(
        StructTypeSymbol* const structSymbol,
        InstanceVarSymbol* const varSymbol,
        const std::vector<SrcLocation>& varUseSrcLocations
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        if (varUseSrcLocations.size() > 1)
        {
            std::for_each(begin(varUseSrcLocations) + 1, end(varUseSrcLocations),
            [&](const SrcLocation& varUseSrcLocation)
            {
                diagnostics.Add(CreateStructVarInitializedMoreThanOnceError(
                    varUseSrcLocation,
                    varUseSrcLocations.front()
                ));
            });
        }

        return Diagnosed<void>{ diagnostics };
    }

    static auto DiagnoseVarsSpecifiedMoreThanOnce(
        StructTypeSymbol* const structSymbol,
        const std::vector<InstanceVarSymbol*>& varSymbols,
        const std::map<InstanceVarSymbol*, std::vector<SrcLocation>>& varSymbolToUseSrcLocationsMap
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        std::for_each(begin(varSymbols), end(varSymbols),
        [&](InstanceVarSymbol* const varSymbol)
        {
            diagnostics.Add(DiagnoseStructConstructionVarSpecifiedMoreThanOnce(
                structSymbol,
                varSymbol,
                varSymbolToUseSrcLocationsMap.at(varSymbol)
            ));
        });

        return Diagnosed<void>{ diagnostics };
    }

    static auto CreateBoundArgs(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        StructTypeSymbol* const structSymbol,
        const std::vector<StructConstructionExprArg>& args
    ) -> Diagnosed<std::vector<StructConstructionExprBoundArg>>
    {
        DiagnosticBag diagnostics{};

        const auto varSymbols = structSymbol->CollectVars();

        auto varSymbolToUseSrcLocationsMap =
            CreateVarSymbolToUseSrcLocationsMap(varSymbols);

        std::vector<StructConstructionExprBoundArg> boundArgs{};
        std::transform(begin(args), end(args), back_inserter(boundArgs),
        [&](const StructConstructionExprArg& arg) -> StructConstructionExprBoundArg
        {
            const auto matchingVarSymbolIt = std::find_if(
                begin(varSymbolToUseSrcLocationsMap),
                end  (varSymbolToUseSrcLocationsMap),
                [&](const std::pair<InstanceVarSymbol* const, const std::vector<SrcLocation>&>& varSymbolAndUseSrcLocationsPair)
                {
                    auto* const varSymbol =
                        varSymbolAndUseSrcLocationsPair.first;

                    return varSymbol->GetName().String == arg.Name.String;
                }
            );

            const bool hasMatchingVarSymbol =
                matchingVarSymbolIt != end(varSymbolToUseSrcLocationsMap);

            if (hasMatchingVarSymbol)
            {
                auto* const varSymbol = matchingVarSymbolIt->first;
                varSymbolToUseSrcLocationsMap.at(varSymbol).push_back(
                    arg.Name.SrcLocation
                );
            }
            else
            {
                diagnostics.Add(CreateStructHasNoVarNamedError(
                    structSymbol,
                    arg.Name
                ));
            }

            auto* const compilation = scope->GetCompilation();

            auto* const varSymbol = hasMatchingVarSymbol ?
                matchingVarSymbolIt->first :
                compilation->ErrorSymbols->GetInstanceVar();

            const auto boundValue = CreateBoundArgValue(scope, arg);

            return StructConstructionExprBoundArg
            {
                varSymbol,
                boundValue,
            };
        });

        diagnostics.Add(DiagnoseInaccessibleVars(
            scope,
            varSymbols,
            varSymbolToUseSrcLocationsMap
        ));
        diagnostics.Add(DiagnoseMissingVars(
            srcLocation,
            structSymbol,
            varSymbols,
            varSymbolToUseSrcLocationsMap
        ));
        diagnostics.Add(DiagnoseVarsSpecifiedMoreThanOnce(
            structSymbol,
            varSymbols,
            varSymbolToUseSrcLocationsMap
        ));

        return Diagnosed{ boundArgs, diagnostics };
    }

    auto StructConstructionExprNode::CreateBound() const -> std::shared_ptr<const StructConstructionExprBoundNode>
    {
        DiagnosticBag diagnostics{};

        const auto expStructSymbol = m_Scope->ResolveStaticSymbol<StructTypeSymbol>(
            m_TypeName
        );
        diagnostics.Add(expStructSymbol);

        const auto boundArgs = [&]() -> std::vector<StructConstructionExprBoundArg>
        {
            if (!expStructSymbol)
            {
                return {};
            }

            const auto dgnBoundArgs = CreateBoundArgs(
                m_TypeName.CreateSrcLocation(),
                GetScope(),
                expStructSymbol.Unwrap(),
                m_Args
            );
            diagnostics.Add(dgnBoundArgs);

            return dgnBoundArgs.Unwrap();
        }();

        auto* const structSymbol = expStructSymbol.UnwrapOr(
            GetCompilation()->ErrorSymbols->GetStructType()
        );

        return std::make_shared<const StructConstructionExprBoundNode>(
            diagnostics,
            GetSrcLocation(),
            GetScope(),
            structSymbol,
            boundArgs
        );
    }

    auto StructConstructionExprNode::CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateBound();
    }
}
