#include "Nodes/Vars/Params/SelfParamVarNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "SpecialIdent.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Symbols/Symbol.hpp"
#include "Ident.hpp"

namespace Ace
{
    SelfParamVarNode::SelfParamVarNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& typeName
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ srcLocation, SpecialIdent::Self },
        m_TypeName{ typeName, std::vector{ TypeNameModifier::Ref } }
    {
    }

    auto SelfParamVarNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SelfParamVarNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto SelfParamVarNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const SelfParamVarNode>
    {
        return std::make_shared<const SelfParamVarNode>(
            m_SrcLocation,
            scope,
            m_TypeName.SymbolName
        );
    }

    auto SelfParamVarNode::CreateBound() const -> Expected<std::shared_ptr<const SelfParamVarBoundNode>>
    {
        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<SelfParamVarSymbol>(
            m_Name
        ).Unwrap();

        return std::make_shared<const SelfParamVarBoundNode>(
            GetSrcLocation(),
            selfSymbol
        );
    }

    auto SelfParamVarNode::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto SelfParamVarNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Function;
    }

    auto SelfParamVarNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto SelfParamVarNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expTypeSymbol = m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        );
        diagnosticBag.Add(expTypeSymbol);

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<SelfParamVarSymbol>(
                m_SrcLocation,
                m_Scope,
                expTypeSymbol.UnwrapOr(GetCompilation()->ErrorTypeSymbol)
            ),
            diagnosticBag,
        };
    }
}
