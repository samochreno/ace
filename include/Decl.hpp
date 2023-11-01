#pragma once

#include <memory>

#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Ident.hpp"

namespace Ace
{
    class ISymbol;

    enum class DeclOrder
    {
        BeforeType,

        Type,
        TypeReimport,
        TypeAlias,

        AfterType,
    };

    class IDecl
    {
    public:
        virtual ~IDecl() = default;

        virtual auto GetSymbolScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetDeclOrder() const -> DeclOrder = 0;
        virtual auto GetDeclSuborder() const -> size_t;
        virtual auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> = 0;
    };

    class IPartialDecl : public virtual IDecl
    {
    public:
        virtual ~IPartialDecl() = default;

        virtual auto GetName() const -> const Ident& = 0;
        virtual auto ContinueCreatingSymbol(
            ISymbol* const symbol
        ) const -> Diagnosed<void> = 0;
    };
}
