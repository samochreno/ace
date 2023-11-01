#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>

#include "SrcLocation.hpp"
#include "Compilation.hpp"
#include "Scope.hpp"
#include "Decl.hpp"
#include "Assert.hpp"

namespace Ace
{
    class ISyntax;
    class TypeParamSyntax;
    class ITypeSymbol;

    class SyntaxChildCollector
    {
    public:
        SyntaxChildCollector() = default;
        ~SyntaxChildCollector() = default;

        template<typename T>
        auto Collect(
            const std::shared_ptr<const T>& syntax
        ) -> SyntaxChildCollector&
        {
            m_Children.push_back(syntax.get());
            return CollectChildren(syntax);
        }
        template<typename T>
        auto Collect(
            const std::optional<std::shared_ptr<const T>>& optSema
        ) -> SyntaxChildCollector&
        {
            if (optSema.has_value())
            {
                Collect(optSema.value());
            }

            return *this;
        }
        template<typename T>
        auto Collect(
            const std::vector<std::shared_ptr<const T>>& syntaxes
        ) -> SyntaxChildCollector&
        {
            std::for_each(begin(syntaxes), end(syntaxes),
            [&](const std::shared_ptr<const T>& syntax)
            {
                Collect(syntax);
            });

            return *this;
        }
        template<typename T>
        auto CollectChildren(
            const std::shared_ptr<const T>& syntax
        ) -> SyntaxChildCollector&
        {
            const auto children = syntax->CollectChildren();
            m_Children.insert(end(m_Children), begin(children), end(children));
            return *this;
        }

        auto Build() const -> std::vector<const ISyntax*>;

    private:
        std::vector<const ISyntax*> m_Children{};
    };

    class ISyntax
    {
    public:
        virtual ~ISyntax() = default;

        virtual auto GetSrcLocation() const -> const SrcLocation& = 0;
        virtual auto GetCompilation() const -> Compilation* final;
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto CollectChildren() const -> std::vector<const ISyntax*> = 0;
    };

    template<typename T>
    class ISemaSyntax : public virtual ISyntax
    {
    public:
        virtual ~ISemaSyntax() = default;

        virtual auto CreateSema() const -> Diagnosed<std::shared_ptr<const T>> = 0;
    };

    class IDeclSyntax :
        public virtual ISyntax,
        public virtual IDecl
    {
    public:
        virtual ~IDeclSyntax() = default;
    };

    class IPartialDeclSyntax :
        public virtual IDeclSyntax,
        public virtual IPartialDecl
    {
    public:
        virtual ~IPartialDeclSyntax() = default;
    };

    auto ResolveTypeParamSymbols(
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams
    ) -> std::vector<ITypeSymbol*>;
}
