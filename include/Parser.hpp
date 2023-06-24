#pragma once

#include <memory>
#include <vector>
#include <string>
#include <utility>

#include "Token.hpp"
#include "Node/All.hpp"
#include "Diagnostics.hpp"
#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "FileBuffer.hpp"
#include "Measured.hpp"

namespace Ace
{
    class ParseToken
    {
    public:
        ParseToken(const std::shared_ptr<const Token>& t_value);
        ~ParseToken() = default;

        auto Unwrap() const -> const Token&;
        operator const std::shared_ptr<const Token>&() const { return m_Value; }

    private:
        std::shared_ptr<const Token> m_Value{};
    };

    struct ParseContext
    {
        ParseContext(
            const std::vector<ParseToken>::const_iterator& t_iterator,
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
        ) -> Expected<std::shared_ptr<const Node::Module>>;

        static auto CreateEmptyAttributes() -> std::vector<std::shared_ptr<const Node::Attribute>>;

        static auto GetOperatorFunctionName(
            const std::shared_ptr<const Token>& t_operatorToken,
            const size_t& t_params
        ) -> Expected<const char*>;

        static auto ParseName(
            const ParseContext& t_context
        ) -> Expected<Measured<std::string>>;
        static auto ParseNestedName(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::string>>>;
        static auto ParseSymbolName(
            const ParseContext& t_context
        ) -> Expected<Measured<SymbolName>>;
        static auto ParseSymbolNameSection(
            const ParseContext& t_context
        ) -> Expected<Measured<SymbolNameSection>>;
        static auto ParseTypeName(
            const ParseContext& t_context,
            const bool& t_doAllowReferences
        ) -> Expected<Measured<TypeName>>;
        static auto ParseTemplateParamNames(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::string>>>;
        static auto ParseImplTemplateParams(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const Node::TemplateParam::Impl>>>>;
        static auto ParseTemplateParams(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const Node::TemplateParam::Normal>>>>;
        static auto ParseTemplateArgs(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<SymbolName>>>;
        static auto ParseModule(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Module>>>;
        static auto ParseImpl(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Impl>>>;
        static auto ParseImplFunction(
            const ParseContext& t_context,
            const SymbolName& t_selfTypeName
        ) -> Expected<Measured<std::shared_ptr<const Node::Function>>>;
        static auto ParseImplFunctionTemplate(
            const ParseContext& t_context,
            const SymbolName& t_selfTypeName
        ) -> Expected<Measured<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseTemplatedImpl(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::TemplatedImpl>>>;
        static auto ParseTemplatedImplFunction(
            const ParseContext& t_context,
            const SymbolName& t_selfTypeName,
            const std::vector<std::shared_ptr<const Node::TemplateParam::Impl>>& t_implTemplateParams
        ) -> Expected<Measured<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseFunction(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Function>>>;
        static auto ParseFunctionTemplate(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseParams(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const Node::Var::Param::Normal>>>>;
        static auto ParseVar(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Var::Normal::Static>>>;
        static auto ParseMemberVar(
            const ParseContext& t_context,
            const size_t& t_index
        ) -> Expected<Measured<std::shared_ptr<const Node::Var::Normal::Instance>>>;
        static auto ParseType(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Type::IBase>>>;
        static auto ParseTypeTemplate(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Template::Type>>>;
        static auto ParseStruct(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Type::Struct>>>;
        static auto ParseStructBody(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const Node::Var::Normal::Instance>>>>;
        static auto ParseStructTemplate(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Template::Type>>>;
        static auto ParseStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::IBase>>>;
        static auto ParseExprStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Expr>>>;
        static auto ParseAssignmentStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Assignment::Normal>>>;
        static auto ParseCompoundAssignmentStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Assignment::Compound>>>;
        static auto ParseVarStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Var>>>;
        static auto ParseKeywordStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::IBase>>>;
        static auto ParseIfStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::If>>>;
        static auto ParseIfBlock(
            const ParseContext& t_context
        ) -> Expected<Measured<std::pair<std::shared_ptr<const Node::Expr::IBase>, std::shared_ptr<const Node::Stmt::Block>>>>;
        static auto ParseElifBlock(
            const ParseContext& t_context
        ) -> Expected<Measured<std::pair<std::shared_ptr<const Node::Expr::IBase>, std::shared_ptr<const Node::Stmt::Block>>>>;
        static auto ParseElseBlock(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Block>>>;
        static auto ParseWhileStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::While>>>;
        static auto ParseReturnStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Return>>>;
        static auto ParseExitStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Exit>>>;
        static auto ParseAssertStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Assert>>>;
        static auto ParseBlockStmt(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Stmt::Block>>>;
        static auto ParseExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::IBase>>>;
        static auto ParseSimpleExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::IBase>>>;
        static auto ParseMemberAccessExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::MemberAccess>>>;
        static auto ParseArgs(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const Node::Expr::IBase>>>>;
        static auto ParsePrimaryExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::IBase>>>;
        static auto ParseExprExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::Expr>>>;
        static auto ParseLiteralExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::Literal>>>;
        static auto ParseLiteralSymbolExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::LiteralSymbol>>>;
        static auto ParseStructConstructionExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::StructConstruction>>>;
        static auto ParseCastExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::Cast>>>;
        static auto ParseAddressOfExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::AddressOf>>>;
        static auto ParseSizeOfExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::SizeOf>>>;
        static auto ParseDerefAsExpr(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Expr::DerefAs>>>;
        static auto ParseAttribute(
            const ParseContext& t_context
        ) -> Expected<Measured<std::shared_ptr<const Node::Attribute>>>;
        static auto ParseAttributes(
            const ParseContext& t_context
        ) -> Expected<Measured<std::vector<std::shared_ptr<const Node::Attribute>>>>;
    };
}
