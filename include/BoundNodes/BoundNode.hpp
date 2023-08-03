#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <optional>

#include "Compilation.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    struct TypeCheckingContext
    {
    };

    struct LoweringContext
    {
    };

    class IBoundNode
    {
    public:
        virtual ~IBoundNode() = default;

        virtual auto GetCompilation() const -> Compilation* final;
        [[deprecated]]
        virtual auto GetDiagnostics() const -> const DiagnosticBag& = 0;
        virtual auto CollectDiagnostics() const -> DiagnosticBag final;
        virtual auto GetSrcLocation() const -> const SrcLocation& = 0;
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto CollectChildren() const -> std::vector<const IBoundNode*> = 0;
    };

    template<typename T>
    class ICloneableWithDiagnosticsBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ICloneableWithDiagnosticsBoundNode() = default;

        virtual auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const T> = 0;
    };

    template<typename TNode, typename TContext = TypeCheckingContext>
    class ITypeCheckableBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ITypeCheckableBoundNode() = default;

        virtual auto GetOrCreateTypeChecked(
            const TContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const TNode>>> = 0;
    };

    template<typename TNode, typename TContext = LoweringContext>
    class ILowerableBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ILowerableBoundNode() = default;

        virtual auto GetOrCreateLowered(
            const TContext& context
        ) const -> MaybeChanged<std::shared_ptr<const TNode>> = 0;
    };

    template<typename T>
    auto AddChildren(
        std::vector<const IBoundNode*>& vec,
        const std::shared_ptr<const T>& node
    ) -> void
    {
        if (!node)
        {
            return;
        }

        vec.push_back(node.get());
        std::vector<const IBoundNode*> childChildren = node->CollectChildren();
        vec.insert(end(vec), begin(childChildren), end(childChildren));
    }

    template<typename T>
    auto AddChildren(
        std::vector<const IBoundNode*>& vec,
        const std::vector<std::shared_ptr<const T>>& nodes
    ) -> void
    {
        std::for_each(begin(nodes), end(nodes),
        [&](const std::shared_ptr<const T>& node)
        {
            vec.push_back(node.get());

            auto children = node->CollectChildren();
            vec.insert(end(vec), begin(children), end(children));
        });
    }
}
