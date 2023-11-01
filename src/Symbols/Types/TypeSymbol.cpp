#include "Symbols/Types/TypeSymbol.hpp"

#include <vector>
#include <optional>

#include "Symbols/Types/Aliases/AliasTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Scope.hpp"
#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "Compilation.hpp"

namespace Ace
{
    static auto IsInstanceOf(
        const ISymbol* const symbol,
        IGenericSymbol* const root
    ) -> bool
    {
        return symbol->GetRoot() == root;
    }

    auto ITypeSymbol::GetUnaliasedType() const -> const ITypeSymbol*
    {
        return dynamic_cast<const ITypeSymbol*>(GetUnaliased());
    }

    auto ITypeSymbol::GetUnaliasedType() -> ITypeSymbol*
    {
        return dynamic_cast<ITypeSymbol*>(GetUnaliased());
    }

    auto ITypeSymbol::DiagnoseCycle() const -> Diagnosed<void>
    {
        return Diagnosed<void>{ DiagnosticBag::Create() };
    }

    auto ITypeSymbol::IsRef() const -> bool
    {
        return IsInstanceOf(
            GetUnaliased(),
            GetCompilation()->GetNatives().Ref.GetSymbol()
        );
    }

    auto ITypeSymbol::GetWithoutRef() -> ITypeSymbol*
    {
        if (!IsRef())
        {
            return this;
        }

        return GetUnaliasedType()->GetTypeArgs().front();
    }

    auto ITypeSymbol::GetWithoutRef() const -> const ITypeSymbol*
    {
        if (!IsRef())
        {
            return this;
        }

        return GetUnaliasedType()->GetTypeArgs().front();
    }

    auto ITypeSymbol::GetWithRef() -> ITypeSymbol*
    {
        ACE_ASSERT(!IsRef());

        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetCompilation()->GetNatives().Ref.GetSymbol(),
            { this }
        );

        auto* const type = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(type);
        return type;
    }

    auto ITypeSymbol::IsStrongPtr() const -> bool
    {
        return IsInstanceOf(
            GetUnaliased(),
            GetCompilation()->GetNatives().StrongPtr.GetSymbol()
        );
    }

    auto ITypeSymbol::IsDynStrongPtr() const -> bool
    {
        return IsInstanceOf(
            GetUnaliased(),
            GetCompilation()->GetNatives().DynStrongPtr.GetSymbol()
        );
    }

    auto ITypeSymbol::IsAnyStrongPtr() const -> bool
    {
        if (IsStrongPtr())
        {
            return true;
        }

        if (IsDynStrongPtr())
        {
            return true;
        }

        return false;
    }

    auto ITypeSymbol::GetWithoutStrongPtr() -> ITypeSymbol*
    {
        if (!IsAnyStrongPtr())
        {
            return this;
        }

        return GetUnaliasedType()->GetTypeArgs().front();
    }

    auto ITypeSymbol::GetWithStrongPtr() -> ITypeSymbol*
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetCompilation()->GetNatives().StrongPtr.GetSymbol(),
            { this }
        );

        auto* const type = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(type);
        return type;
    }

    auto ITypeSymbol::GetWithDynStrongPtr() -> ITypeSymbol*
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetCompilation()->GetNatives().DynStrongPtr.GetSymbol(),
            { this }
        );

        auto* const type = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(type);
        return type;
    }

    auto ITypeSymbol::GetWithAutoStrongPtr() -> ITypeSymbol*
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        auto* const sizedSelf = dynamic_cast<ISizedTypeSymbol*>(GetUnaliased());

        auto* const strongPtr = sizedSelf ?
            GetCompilation()->GetNatives().StrongPtr.GetSymbol() :
            GetCompilation()->GetNatives().DynStrongPtr.GetSymbol();

        auto* const symbol =
            Scope::ForceCollectGenericInstance(strongPtr, { this });

        auto* const type = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(type);
        return type;
    }

    auto ITypeSymbol::IsWeakPtr() const -> bool
    {
        return IsInstanceOf(
            GetUnaliased(),
            GetCompilation()->GetNatives().WeakPtr.GetSymbol()
        );
    }

    auto ITypeSymbol::GetWithoutWeakPtr() -> ITypeSymbol*
    {
        if (!IsWeakPtr())
        {
            return this;
        }

        return GetUnaliasedType()->GetTypeArgs().front();
    }

    auto ITypeSymbol::GetWithWeakPtr() -> ITypeSymbol*
    {
        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetCompilation()->GetNatives().WeakPtr.GetSymbol(),
            { this }
        );

        auto* const type = dynamic_cast<ITypeSymbol*>(symbol);
        ACE_ASSERT(type);
        return type;
    }

    auto ITypeSymbol::GetDerefed() -> ITypeSymbol*
    {
        if (IsRef())
        {
            return GetWithoutRef()->GetDerefed();
        }

        if (IsAnyStrongPtr())
        {
            return GetWithoutStrongPtr()->GetDerefed();
        }

        return GetUnaliasedType();
    }
}
