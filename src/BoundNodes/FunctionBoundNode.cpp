#include "BoundNodes/FunctionBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    FunctionBoundNode::FunctionBoundNode(
        const SrcLocation& srcLocation,
        FunctionSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
        const std::optional<const std::shared_ptr<const SelfParamVarBoundNode>>& optSelf,
        const std::vector<std::shared_ptr<const NormalParamVarBoundNode>>& params,
        const std::optional<std::shared_ptr<const BlockStmtBoundNode>>& optBody
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes },
        m_OptSelf{ optSelf },
        m_Params{ params },
        m_OptBody{ optBody }
    {
    }

    auto FunctionBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FunctionBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto FunctionBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);

        if (m_OptSelf.has_value())
        {
            AddChildren(children, m_OptSelf.value());
        }

        AddChildren(children, m_Params);

        if (m_OptBody.has_value())
        {
            AddChildren(children, m_OptBody.value());
        }

        return children;
    }

    auto FunctionBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const FunctionBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const AttributeBoundNode>> checkedAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            std::back_inserter(checkedAttributes),
            [&](const std::shared_ptr<const AttributeBoundNode>& attribute)
            {
                return diagnostics.Collect(attribute->CreateTypeChecked({}));
            }
        );

        std::optional<std::shared_ptr<const SelfParamVarBoundNode>> checkedOptSelf{};
        if (m_OptSelf.has_value())
        {
            checkedOptSelf =
                diagnostics.Collect(m_OptSelf.value()->CreateTypeChecked({}));
        }

        std::vector<std::shared_ptr<const NormalParamVarBoundNode>> checkedParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            std::back_inserter(checkedParams),
            [&](const std::shared_ptr<const NormalParamVarBoundNode>& param)
            {
                return diagnostics.Collect(param->CreateTypeChecked({}));
            }
        );

        std::optional<std::shared_ptr<const BlockStmtBoundNode>> checkedOptBody{};
        if (m_OptBody.has_value())
        {
            checkedOptBody = diagnostics.Collect(m_OptBody.value()->CreateTypeChecked({
                m_Symbol->GetType()
            }));
        }

        if (
            (checkedAttributes == m_Attributes) &&
            (checkedOptSelf == m_OptSelf) &&
            (checkedParams == m_Params) &&
            (checkedOptBody == m_OptBody)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const FunctionBoundNode>(
                GetSrcLocation(),
                m_Symbol,
                checkedAttributes,
                checkedOptSelf,
                checkedParams,
                checkedOptBody
            ),
            std::move(diagnostics),
        };
    }

    auto FunctionBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const FunctionBoundNode>
    {
        std::vector<std::shared_ptr<const AttributeBoundNode>> loweredAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            std::back_inserter(loweredAttributes),
            [&](const std::shared_ptr<const AttributeBoundNode>& attribute)
            {
                return attribute->CreateLowered({});
            }
        );
        
        const auto loweredOptSelf = m_OptSelf.has_value() ?
            std::optional{ m_OptSelf.value()->CreateLowered({}) } :
            std::nullopt;

        std::vector<std::shared_ptr<const NormalParamVarBoundNode>> loweredParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            std::back_inserter(loweredParams),
            [&](const std::shared_ptr<const NormalParamVarBoundNode>& param)
            {
                return param->CreateLowered({});
            }
        );

        const auto loweredOptBody = m_OptBody.has_value() ?
            std::optional{ m_OptBody.value()->CreateLowered({}) } :
            std::nullopt;

        if (
            (loweredAttributes == m_Attributes) && 
            (loweredOptSelf == m_OptSelf) &&
            (loweredParams == m_Params) && 
            (loweredOptBody == m_OptBody)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const FunctionBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            loweredAttributes,
            loweredOptSelf,
            loweredParams,
            loweredOptBody
        )->CreateLowered({});
    }

    auto FunctionBoundNode::GetSymbol() const -> FunctionSymbol*
    {
        return m_Symbol;
    }

    auto FunctionBoundNode::GetBody() const -> std::optional<std::shared_ptr<const BlockStmtBoundNode>>
    {
        return m_OptBody;
    }
}
