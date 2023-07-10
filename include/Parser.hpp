#pragma once

#include <memory>
#include <vector>
#include <string>
#include <utility>

#include "Token.hpp"
#include "Nodes/All.hpp"
#include "Diagnostics.hpp"
#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "FileBuffer.hpp"
#include "Measured.hpp"
#include "Identifier.hpp"

namespace Ace
{
    class ParseToken
    {
    public:
        ParseToken(const std::shared_ptr<const Token>& t_value);
        ~ParseToken() = default;

        auto Unwrap() const -> const Token&;
        operator const std::shared_ptr<const Token>&() const
        {
            return m_Value;
        }

    private:
        std::shared_ptr<const Token> m_Value{};
    };

    enum class ReferenceParsingKind
    {
        Allow,
        Disallow,
    };

    struct ParseContext
    {
        ParseContext(
            const std::vector<ParseToken>::const_iterator t_iterator,
            const std::shared_ptr<Scope>& t_scope
        ) : Iterator{ t_iterator },
            Scope{ t_scope }
        {
        }

        const std::vector<ParseToken>::const_iterator Iterator{};
        std::shared_ptr<Ace::Scope> Scope{};
    };

    class Parser
    {
    public:
        Parser() = delete;

        static auto ParseAST(
            const FileBuffer* const t_fileBuffer,
            const std::vector<std::shared_ptr<const Token>>& t_tokens
        ) -> Expected<std::shared_ptr<const ModuleNode>>;

        static auto CreateEmptyAttributes() -> std::vector<std::shared_ptr<const AttributeNode>>;

        static auto GetOperatorFunctionName(
            const std::shared_ptr<const Token>& t_operatorToken,
            const size_t t_params
        ) -> Expected<const char*>;

        static auto ParseName(
            const ParseContext& t_context
        ) -> Expected<Measured<Identifier>>;
        static auto ParseNestedName(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<Identifier>>>;
        static auto ParseSymbolName(
            const ParseContext& t_context
        ) -> Expected<Measured<SymbolName>>;
        static auto ParseSymbolNameSection(
            const ParseContext& t_context
        ) -> Expected<Measured<SymbolNameSection>>;
        static auto ParseTypeName(
            const ParseContext& t_context,
            const ReferenceParsingKind t_referenceParsingKind
        ) -> Expected<Measured<TypeName>>;
        static auto ParseTemplateParamNames(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<Identifier>>>;
        static auto ParseImplTemplateParams(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const ImplTemplateParamNode>>>>;
        static auto ParseTemplateParams(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>>;
        static auto ParseTemplateArgs(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<SymbolName>>>;
        static auto ParseModule(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const ModuleNode>>>;
        static auto ParseImpl(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const ImplNode>>>;
        static auto ParseImplFunction(
            const ParseContext& t_context,
            const SymbolName& t_selfTypeName
        ) -> Expected<Measured<std::shared_ptr<const FunctionNode>>>;
        static auto ParseImplFunctionTemplate(
            const ParseContext& t_context,
            const SymbolName& t_selfTypeName
        ) -> Expected<Measured<std::shared_ptr<const FunctionTemplateNode>>>;
        static auto ParseTemplatedImpl(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const TemplatedImplNode>>>;
        static auto ParseTemplatedImplFunction(
            const ParseContext& t_context,
            const SymbolName& t_selfTypeName,
            const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& t_implTemplateParams
        ) -> Expected<Measured<std::shared_ptr<const FunctionTemplateNode>>>;
        static auto ParseFunction(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const FunctionNode>>>;
        static auto ParseFunctionTemplate(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const FunctionTemplateNode>>>;
        static auto ParseParam(
            const ParseContext& t_context,
            const size_t t_index
        ) -> Expected<Measured<std::shared_ptr<const NormalParamVarNode>>>;
        static auto ParseParams(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const NormalParamVarNode>>>>;
        static auto ParseVar(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const StaticVarNode>>>;
        static auto ParseMemberVar(
            const ParseContext& t_context,
            const size_t t_index
        ) -> Expected<Measured<std::shared_ptr<const InstanceVarNode>>>;
        static auto ParseType(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const ITypeNode>>>;
        static auto ParseTypeTemplate(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const TypeTemplateNode>>>;
        static auto ParseStruct(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const StructTypeNode>>>;
        static auto ParseStructBody(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const InstanceVarNode>>>>;
        static auto ParseStructTemplate(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const TypeTemplateNode>>>;
        static auto ParseStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const IStmtNode>>>;
        static auto ParseExprStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const ExprStmtNode>>>;
        static auto ParseAssignmentStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const NormalAssignmentStmtNode>>>;
        static auto ParseCompoundAssignmentStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const CompoundAssignmentStmtNode>>>;
        static auto ParseVarStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const VarStmtNode>>>;
        static auto ParseKeywordStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const IStmtNode>>>;
        static auto ParseIfStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const IfStmtNode>>>;
        static auto ParseIfBlock(
            const ParseContext& t_context
        ) -> Expected<Measured<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>>;
        static auto ParseElifBlock(
            const ParseContext& t_context
        ) -> Expected<Measured<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>>;
        static auto ParseElseBlock(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const BlockStmtNode>>>;
        static auto ParseWhileStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const WhileStmtNode>>>;
        static auto ParseReturnStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const ReturnStmtNode>>>;
        static auto ParseExitStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const ExitStmtNode>>>;
        static auto ParseAssertStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const AssertStmtNode>>>;
        static auto ParseBlockStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const BlockStmtNode>>>;
        static auto ParseExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const IExprNode>>>;
        static auto ParseSimpleExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const IExprNode>>>;
        static auto ParseMemberAccessExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const MemberAccessExprNode>>>;
        static auto ParseArgs(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const IExprNode>>>>;
        static auto ParsePrimaryExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const IExprNode>>>;
        static auto ParseExprExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const ExprExprNode>>>;
        static auto ParseLiteralExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const LiteralExprNode>>>;
        static auto ParseLiteralSymbolExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const LiteralSymbolExprNode>>>;
        static auto ParseStructConstructionExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const StructConstructionExprNode>>>;
        static auto ParseCastExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const CastExprNode>>>;
        static auto ParseAddressOfExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const AddressOfExprNode>>>;
        static auto ParseSizeOfExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const SizeOfExprNode>>>;
        static auto ParseDerefAsExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const DerefAsExprNode>>>;
        static auto ParseAttribute(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const AttributeNode>>>;
        static auto ParseAttributes(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const AttributeNode>>>>;
    };
}
