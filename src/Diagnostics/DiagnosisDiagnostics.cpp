#include "Diagnostics/DiagnosisDiagnostics.hpp"

#include <memory>
#include <string>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/All.hpp"

namespace Ace
{
    auto CreateUnimplementedTraitFunctionError(
        TraitImplSymbol* const impl,
        ISymbol* const function
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            impl->GetName().SrcLocation,
            "unimplemented function `" + function->GetName().String + "`"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            function->GetName().SrcLocation,
            "function declaration"
        );

        return group;
    }

    auto CreateUnimplementedSupertraitError(
        SupertraitSymbol* const supertrait,
        TraitImplSymbol* impl
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unimplemented supertrait `" +
            supertrait->GetTrait()->CreateDisplayName() + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            impl->GetName().SrcLocation,
            message
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            supertrait->GetName().SrcLocation,
            "supertrait declaration"
        );

        return group;
    }

    auto CreateMismatchedTraitImplTypeError(
        ISymbol* const symbol,
        const SrcLocation& functionTypeSrcLocation,
        const SrcLocation& prototypeTypeSrcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            functionTypeSrcLocation,
            "mismatched " + symbol->CreateTypeNoun().String + " type"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            prototypeTypeSrcLocation,
            symbol->CreateTypeNoun().String + " declaration"
        );
        
        return group;
    }

    auto CreateMismatchedTraitImplFunctionParamCountError(
        PrototypeSymbol* const prototype,
        FunctionSymbol* const function
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            function->GetName().SrcLocation,
            "mismatched parameter count"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            prototype->GetName().SrcLocation,
            "function declaration"
        );

        return group;
    }

    auto CreateMismatchedTraitImplFunctionTypeParamCountError(
        PrototypeSymbol* const prototype,
        FunctionSymbol* const function
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            function->GetName().SrcLocation,
            "mismatched type parameter count"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            prototype->GetName().SrcLocation,
            "function declaration"
        );

        return group;
    }

    auto CreateInherentFunctionRedeclarationError(
        FunctionSymbol* const originalFunction,
        FunctionSymbol* const redeclaredFunction
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            redeclaredFunction->GetName().SrcLocation,
            "inherent function redeclaration"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            originalFunction->GetName().SrcLocation,
            "previous declaration"
        );

        return group;
    }

    auto CreateFunctionIsNotMemberOfTraitError(
        FunctionSymbol* const function,
        TraitTypeSymbol* const traitType
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            function->GetName().SrcLocation,
            "not a member of trait `" + traitType->GetName().String + "`"
        );

        return group;
    }

    auto CreateImplHasStricterConstraintsThanPrototypeError(
        PrototypeSymbol* const prototype,
        FunctionSymbol* const function
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const auto functionConstraints = function->CollectConstraints();
        const SrcLocation functionSrcLocation
        {
            functionConstraints.front()->GetName().SrcLocation,
            functionConstraints.back ()->GetName().SrcLocation,
        };
        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            functionSrcLocation,
            "implementation has stricter constraints than prototype"
        );

        const auto prototypeConstraints = prototype->CollectConstraints();
        const SrcLocation prototypeSrcLocation
        {
            prototypeConstraints.front()->GetName().SrcLocation,
            prototypeConstraints.back ()->GetName().SrcLocation,
        };
        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            prototypeSrcLocation,
            "prototype declaration"
        );

        return group;
    }

    auto CreateOverlappingInherentImplSymbolError(
        ISymbol* const overlappingSymbol,
        ISymbol* const symbol
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "overlapping " + overlappingSymbol->CreateTypeNoun().String +
            " declaration";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            overlappingSymbol->GetName().SrcLocation,
            message
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            symbol->GetName().SrcLocation,
            "previous declaration"
        );

        return group;
    }

    auto CreateNotAllControlPathsRetError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "not all control paths return a value"
        );

        return group;
    }

    auto CreateOrphanInherentImplError(
        InherentImplSymbol* const impl
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            impl->GetName().SrcLocation,
            "inherent implementation outside of package of type"
        );

        return group;
    }

    auto CreateOrphanTraitImplError(
        TraitImplSymbol* const impl
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            impl->GetName().SrcLocation,
            "trait implementation outside of package of both trait and type"
        );

        return group;
    }

    auto CreateConcreteConstraintError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "concrete constraint"
        );

        return group;
    }
}
