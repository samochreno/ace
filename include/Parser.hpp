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
#include "Compilation.hpp"

namespace Ace
{
    template<typename T>
    struct ParseData
    {
        ParseData()
            : Length{ 0 }
        {
        }

        template<typename T_ = T, std::enable_if<std::is_copy_constructible_v<T>>* = nullptr>
        ParseData(const T& t_value, const size_t& t_length)
            : Value{ t_value }, Length{ t_length }
        {
        }

        ParseData(T&& t_value, const size_t& t_length)
            : Value{ std::move(t_value) }, Length{ t_length }
        {
        }

        T Value;
        size_t Length;
    };

    class TokenEntry
    {
    public:
        TokenEntry(
            const std::shared_ptr<const Token>& t_value
        ) : m_Value{ t_value }
        {
        }
        ~TokenEntry() = default;

        auto Unwrap() const -> const Token& { return *m_Value.get(); }
        operator const std::shared_ptr<const Token>&() const { return m_Value; }

    private:
        std::shared_ptr<const Token> m_Value{};
    };

    struct ParseContext
    {
        ParseContext(
            const std::vector<TokenEntry>::const_iterator& t_iterator,
            const std::shared_ptr<Scope>& t_scope
        ) : Iterator{ t_iterator },
            Scope{ t_scope }
        {
        }

        const std::vector<TokenEntry>::const_iterator Iterator{};
        std::shared_ptr<Ace::Scope> Scope{};
    };

    class Parser
    {
    public:
        Parser() = delete;

        static auto ParseAST(
            const Compilation* const t_compilation, 
            const std::vector<std::shared_ptr<const Token>>& t_tokens
        ) -> Expected<std::shared_ptr<const Node::Module>>;

        static auto CreateEmptyAttributes() -> std::vector<std::shared_ptr<const Node::Attribute>>;

        static auto GetOperatorFunctionName(
            const std::shared_ptr<const Token>& t_operatorToken,
            const size_t& t_parameters
        ) -> Expected<const char*>;

        static auto ParseName(const ParseContext& t_context) -> Expected<ParseData<std::string>>;
        static auto ParseNestedName(const ParseContext& t_context) -> Expected<ParseData<std::vector<std::string>>>;
        static auto ParseSymbolName(const ParseContext& t_context) -> Expected<ParseData<SymbolName>>;
        static auto ParseSymbolNameSection(const ParseContext& t_context) -> Expected<ParseData<SymbolNameSection>>;
        static auto ParseTypeName(const ParseContext& t_context, const bool& t_doAllowReferences) -> Expected<ParseData<TypeName>>;
        static auto ParseTemplateParameterNames(const ParseContext& t_context) -> Expected<ParseData<std::vector<std::string>>>;
        static auto ParseImplTemplateParameters(const ParseContext& t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>>>>;
        static auto ParseTemplateParameters(const ParseContext& t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>>>>;
        static auto ParseTemplateArguments(const ParseContext& t_context) -> Expected<ParseData<std::vector<SymbolName>>>;
        static auto ParseModule(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Module>>>;
        static auto ParseImpl(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Impl>>>;
        static auto ParseImplFunction(const ParseContext& t_context, const SymbolName& t_selfTypeName) -> Expected<ParseData<std::shared_ptr<const Node::Function>>>;
        static auto ParseImplFunctionTemplate(const ParseContext& t_context, const SymbolName& t_selfTypeName) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseTemplatedImpl(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::TemplatedImpl>>>;
        static auto ParseTemplatedImplFunction(const ParseContext& t_context, const SymbolName& t_selfTypeName, const std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>>& t_implTemplateParameters) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseFunction(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Function>>>;
        static auto ParseFunctionTemplate(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseParameters(const ParseContext& t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Variable::Parameter::Normal>>>>;
        static auto ParseVariable(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Variable::Normal::Static>>>;
        static auto ParseMemberVariable(const ParseContext& t_context, const size_t& t_index) -> Expected<ParseData<std::shared_ptr<const Node::Variable::Normal::Instance>>>;
        static auto ParseType(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Type::IBase>>>;
        static auto ParseTypeTemplate(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Type>>>;
        static auto ParseStruct(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Type::Struct>>>;
        static auto ParseStructBody(const ParseContext& t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>>>>;
        static auto ParseStructTemplate(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Type>>>;
        static auto ParseStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::IBase>>>;
        static auto ParseExpressionStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Expression>>>;
        static auto ParseAssignmentStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assignment::Normal>>>;
        static auto ParseCompoundAssignmentStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assignment::Compound>>>;
        static auto ParseVariableStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Variable>>>;
        static auto ParseKeywordStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::IBase>>>;
        static auto ParseIfStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::If>>>;
        static auto ParseIfBlock(const ParseContext& t_context) -> Expected<ParseData<std::pair<std::shared_ptr<const Node::Expression::IBase>, std::shared_ptr<const Node::Statement::Block>>>>;
        static auto ParseElifBlock(const ParseContext& t_context) -> Expected<ParseData<std::pair<std::shared_ptr<const Node::Expression::IBase>, std::shared_ptr<const Node::Statement::Block>>>>;
        static auto ParseElseBlock(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Block>>>;
        static auto ParseWhileStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::While>>>;
        static auto ParseReturnStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Return>>>;
        static auto ParseExitStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Exit>>>;
        static auto ParseAssertStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assert>>>;
        static auto ParseBlockStatement(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Block>>>;
        static auto ParseExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>;
        static auto ParseSimpleExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>;
        static auto ParseMemberAccessExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::MemberAccess>>>;
        static auto ParseArguments(const ParseContext& t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Expression::IBase>>>>;
        static auto ParsePrimaryExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>;
        static auto ParseExpressionExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Expression>>>;
        static auto ParseLiteralExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Literal>>>;
        static auto ParseLiteralSymbolExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::LiteralSymbol>>>;
        static auto ParseStructConstructionExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::StructConstruction>>>;
        static auto ParseCastExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Cast>>>;
        static auto ParseAddressOfExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::AddressOf>>>;
        static auto ParseSizeOfExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::SizeOf>>>;
        static auto ParseDerefAsExpression(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::DerefAs>>>;
        static auto ParseAttribute(const ParseContext& t_context) -> Expected<ParseData<std::shared_ptr<const Node::Attribute>>>;
        static auto ParseAttributes(const ParseContext& t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Attribute>>>>;
    };
}
