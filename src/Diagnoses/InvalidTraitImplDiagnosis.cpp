#include "Diagnoses/InvalidTraitImplDiagnosis.hpp"

#include <vector>
#include <set>

#include "Compilation.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/DiagnosisDiagnostics.hpp"
#include "Symbols/GenericSymbol.hpp"
#include "Symbols/Impls/TraitImplSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/PrototypeSymbol.hpp"

namespace Ace
{
    static auto DoTypeParamTypesMatch(
        ITypeSymbol* const type,
        ITypeSymbol* const prototypeType
    ) -> bool
    {
        auto* const typeParamType = dynamic_cast<TypeParamTypeSymbol*>(type);
        if (!typeParamType)
        {
            return false;
        }

        auto* const prototypeTypeParamType =
            dynamic_cast<TypeParamTypeSymbol*>(prototypeType);
        if (!prototypeTypeParamType)
        {
            return false;
        }

        return
            typeParamType->GetIndex() ==
            prototypeTypeParamType->GetIndex();
    }

    struct TraitImplFunctionSymbolContext
    {
        TraitImplSymbol* Impl{};
        PrototypeSymbol* Prototype{};
        FunctionSymbol* Function{};
    };

    static auto DoSelfTypesMatch(
        const TraitImplFunctionSymbolContext& context,
        ITypeSymbol* const type,
        ITypeSymbol* const prototypeType
    ) -> bool
    {
        const auto optSelfType = context.Function->CollectSelfType();
        const auto optPrototypeSelfType = context.Prototype->CollectSelfType();

        return
            optSelfType.has_value() &&
            optPrototypeSelfType.has_value() &&
            (type == optSelfType.value()->GetUnaliased()) &&
            (prototypeType == optPrototypeSelfType.value()->GetUnaliased());
    }
    
    static auto DoTypesMatch(
        const TraitImplFunctionSymbolContext& context,
        ITypeSymbol* type,
        ITypeSymbol* prototypeType
    ) -> bool
    {
        type = type->GetUnaliasedType();
        prototypeType = prototypeType->GetUnaliasedType();

        if (type == prototypeType)
        {
            return true;
        }

        if (DoSelfTypesMatch(context, type, prototypeType))
        {
            return true;
        }

        if (DoTypeParamTypesMatch(type, prototypeType))
        {
            return true;
        }

        if (type->GetTypeArgs().empty())
        {
            return false;
        }

        if (prototypeType->GetTypeArgs().empty())
        {
            return false;
        }

        if (
            (type->GetRoot() != prototypeType->GetRoot()) &&
            (!type->IsStrongPtr() || !prototypeType->IsDynStrongPtr())
            )
        {
            return false;
        }

        if (type->GetTypeArgs().size() != prototypeType->GetTypeArgs().size())
        {
            return false;
        }

        const bool doTypeArgsMatch = std::equal(
            begin(type->GetTypeArgs()),
            end  (type->GetTypeArgs()),
            begin(prototypeType->GetTypeArgs()),
            end  (prototypeType->GetTypeArgs()),
            [&](ITypeSymbol* const typeArg, ITypeSymbol* const prototypeTypeArg)
            {
                return DoTypesMatch(context, typeArg, prototypeTypeArg);
            }
        );
        return doTypeArgsMatch;
    }

    static auto DiagnoseMismatchedTraitImplType(
        const TraitImplFunctionSymbolContext& context,
        ISymbol* const symbol,
        const SrcLocation& prototypeTypeSrcLocation,
        const SrcLocation& functionTypeSrcLocation,
        ITypeSymbol* const prototypeTypeSymbol,
        ITypeSymbol* const functionTypeSymbol
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto doTypesMatch =
            DoTypesMatch(context, functionTypeSymbol, prototypeTypeSymbol);
        if (!doTypesMatch)
        {
            diagnostics.Add(CreateMismatchedTraitImplTypeError(
                symbol,
                prototypeTypeSrcLocation,
                functionTypeSrcLocation
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseMismatchedTraitFunctionParam(
        const TraitImplFunctionSymbolContext& context,
        IParamVarSymbol* const prototypeParamSymbol,
        IParamVarSymbol* const functionParamSymbol
    ) -> Diagnosed<void>
    {
        return DiagnoseMismatchedTraitImplType(
            context,
            functionParamSymbol,
            prototypeParamSymbol->GetName().SrcLocation,
            functionParamSymbol->GetName().SrcLocation,
            prototypeParamSymbol->GetType(),
            functionParamSymbol->GetType()
        );
    }

    static auto DiagnoseMismatchedTraitFunctionParams(
        const TraitImplFunctionSymbolContext& context
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto prototypeParamSymbols =
            context.Prototype->CollectAllParams();
        const auto functionParamSymbols =
            context.Function->CollectAllParams();

        if (prototypeParamSymbols.size() != functionParamSymbols.size())
        {
            diagnostics.Add(CreateMismatchedTraitImplFunctionParamCountError(
                context.Prototype,
                context.Function
            ));
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        for (size_t i = 0; i < prototypeParamSymbols.size(); i++)
        {
            diagnostics.Collect(DiagnoseMismatchedTraitFunctionParam(
                context,
                prototypeParamSymbols.at(i),
                functionParamSymbols.at(i)
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseMismatchedTraitFunctionTypeParams(
        const TraitImplFunctionSymbolContext& context
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (
            context.Prototype->CollectTypeParams().size() !=
            context.Function->CollectTypeParams().size()
            )
        {
            diagnostics.Add(CreateMismatchedTraitImplFunctionTypeParamCountError(
                context.Prototype,
                context.Function
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DoPrototypeConstraintsContain(
        const TraitImplFunctionSymbolContext& context,
        const std::vector<ConstraintSymbol*>& prototypeConstraints,
        ConstraintSymbol* const constraint
    ) -> bool
    {
        std::vector<TraitTypeSymbol*> prototypeTraits{};
        std::for_each(
            begin(prototypeConstraints),
            end  (prototypeConstraints),
            [&](ConstraintSymbol* const prototypeConstraint)
            {
                const auto doTypesMatch = DoTypesMatch(
                    context,
                    prototypeConstraint->GetType(),
                    constraint->GetType()
                );
                if (!doTypesMatch)
                {
                    return;
                }

                prototypeTraits.insert(
                    end(prototypeTraits),
                    begin(prototypeConstraint->GetTraits()),
                    end  (prototypeConstraint->GetTraits())
                );
            }
        );

        const auto unmatchedTraitIt = std::find_if_not(
            begin(constraint->GetTraits()),
            end  (constraint->GetTraits()),
            [&](TraitTypeSymbol* const trait)
            {
                const bool didMatch = std::find_if(
                    begin(prototypeTraits),
                    end  (prototypeTraits),
                    [&](TraitTypeSymbol* const prototypeTrait)
                    {
                        return DoTypesMatch(context, trait, prototypeTrait);
                    }
                ) != end(prototypeTraits);

                return didMatch;
            }
        );
        return unmatchedTraitIt == end(constraint->GetTraits());
    }

    static auto AreImplConstraintsStricterThanPrototype(
        const TraitImplFunctionSymbolContext& context
    ) -> bool 
    {
        const auto prototypeConstraints =
            context.Prototype->CollectConstraints();
        const auto functionConstraints =
            context.Function->CollectConstraints();

        const auto stricterConstraintIt = std::find_if_not(
            begin(functionConstraints),
            end  (functionConstraints),
            [&](ConstraintSymbol* const constraint)
            {
                return DoPrototypeConstraintsContain(
                    context,
                    prototypeConstraints,
                    constraint
                );
            }
        );
        return stricterConstraintIt != end(functionConstraints);
    }

    static auto DiagnoseImplHasStricterConstraintsThanPrototype(
        const TraitImplFunctionSymbolContext& context
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (AreImplConstraintsStricterThanPrototype(context))
        {
            diagnostics.Add(CreateImplHasStricterConstraintsThanPrototypeError(
                context.Prototype,
                context.Function
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseMismatchedTraitFunction(
        const TraitImplFunctionSymbolContext& context
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        diagnostics.Collect(DiagnoseMismatchedTraitImplType(
            context,
            context.Function,
            context.Prototype->GetName().SrcLocation,
            context.Function->GetName().SrcLocation,
            context.Prototype->GetType(),
            context.Function->GetType()
        ));
        diagnostics.Collect(DiagnoseMismatchedTraitFunctionParams(context));
        diagnostics.Collect(DiagnoseMismatchedTraitFunctionTypeParams(context));

        diagnostics.Collect(
            DiagnoseImplHasStricterConstraintsThanPrototype(context)
        );

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseInvalidTraitFunction(
        TraitImplSymbol* const implSymbol,
        PrototypeSymbol* const prototypeSymbol,
        const std::set<FunctionSymbol*>& functionSymbolSet
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto functionSymbolIt = std::find_if(
            begin(functionSymbolSet),
            end  (functionSymbolSet),
            [&](FunctionSymbol* const functionSymbol)
            {
                return
                    prototypeSymbol->GetName().String ==
                    functionSymbol->GetName().String;
            }
        );
        if (functionSymbolIt == end(functionSymbolSet))
        {
            diagnostics.Add(CreateUnimplementedTraitFunctionError(
                implSymbol,
                prototypeSymbol
            ));
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        diagnostics.Collect(DiagnoseMismatchedTraitFunction({
            implSymbol,
            prototypeSymbol,
            *functionSymbolIt
        }));

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    template<typename T>
    static auto CollectRootSymbolSet(
        const std::shared_ptr<Scope>& scope
    ) -> std::set<T*>
    {
        std::set<T*> symbolSet{};
    
        const auto symbols = scope->CollectAllSymbols();
        std::for_each(begin(symbols), end(symbols),
        [&](ISymbol* symbol)
        {
            if (auto* const rootSymbol = dynamic_cast<T*>(symbol->GetRoot()))
            {
                symbolSet.insert(rootSymbol);
            }
        });

        return symbolSet;
    }

    static auto DiagnoseInvalidTraitFunctions(
        TraitImplSymbol* const implSymbol
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto prototypeSymbolSet = CollectRootSymbolSet<PrototypeSymbol>(
            implSymbol->GetTrait()->GetPrototypeScope()
        );

        auto functionSymbolSet = CollectRootSymbolSet<FunctionSymbol>(
            implSymbol->GetBodyScope()
        );

        std::for_each(begin(prototypeSymbolSet), end(prototypeSymbolSet),
        [&](PrototypeSymbol* const prototypeSymbol)
        {
            diagnostics.Collect(DiagnoseInvalidTraitFunction(
                implSymbol,
                prototypeSymbol,
                functionSymbolSet
            ));
        });

        std::for_each(begin(prototypeSymbolSet), end(prototypeSymbolSet),
        [&](PrototypeSymbol* const prototypeSymbol)
        {
            const auto matchingFunctionSymbolIt = std::find_if(
                begin(functionSymbolSet),
                end  (functionSymbolSet),
                [&](FunctionSymbol* const functionSymbol)
                {
                    return
                        prototypeSymbol->GetName().String ==
                        functionSymbol->GetName().String;
                }
            );

            if (matchingFunctionSymbolIt != end(functionSymbolSet))
            {
                functionSymbolSet.erase(matchingFunctionSymbolIt);
            }
        });

        std::for_each(begin(functionSymbolSet), end(functionSymbolSet),
        [&](FunctionSymbol* const functionSymbol)
        {
            diagnostics.Add(CreateFunctionIsNotMemberOfTraitError(
                functionSymbol,
                implSymbol->GetTrait()
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto DiagnoseInvalidTraitImpls(
        Compilation* const compilation
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto& scope = compilation->GetPackageBodyScope();

        const auto implSymbols =
            scope->CollectSymbolsRecursive<TraitImplSymbol>();

        std::for_each(begin(implSymbols), end(implSymbols),
        [&](TraitImplSymbol* const implSymbol)
        {
            diagnostics.Collect(DiagnoseInvalidTraitFunctions(implSymbol));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
