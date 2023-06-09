#pragma once

#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "Token.hpp"
#include "Node/All.hpp"
#include "Diagnostics.hpp"
#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Compilation.hpp"

namespace Ace::Parsing
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

    struct Context
    {
        Context(
            const std::vector<Token>::const_iterator& t_iterator,
            const std::shared_ptr<Scope>& t_scope
        ) : Iterator{ t_iterator },
            Scope{ t_scope }
        {
        }
        ~Context() = default;

        const std::vector<Token>::const_iterator Iterator{};
        std::shared_ptr<Ace::Scope> Scope{};
    };

    class Parser
    {
    public:
        Parser() = delete;

        static auto ParseAST(
            const Compilation* const t_compilation, 
            std::vector<Token>&& t_tokens
        ) -> Expected<std::shared_ptr<const Node::Module>>;

        static auto CreateEmptyAttributes() -> std::vector<std::shared_ptr<const Node::Attribute>>;

        static auto GetOperatorFunctionName(
            const Token& t_operatorToken,
            const size_t& t_parameters
        ) -> Expected<const char*>;

        static auto ParseName(Context t_context) -> Expected<ParseData<std::string>>;
        static auto ParseNestedName(Context t_context) -> Expected<ParseData<std::vector<std::string>>>;
        static auto ParseSymbolName(Context t_context) -> Expected<ParseData<SymbolName>>;
        static auto ParseSymbolNameSection(Context t_context) -> Expected<ParseData<SymbolNameSection>>;
        static auto ParseTypeName(Context t_context, const bool& t_doAllowReferences) -> Expected<ParseData<TypeName>>;
        static auto ParseTemplateParameterNames(Context t_context) -> Expected<ParseData<std::vector<std::string>>>;
        static auto ParseImplTemplateParameters(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>>>>;
        static auto ParseTemplateParameters(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>>>>;
        static auto ParseTemplateArguments(Context t_context) -> Expected<ParseData<std::vector<SymbolName>>>;
        static auto ParseModule(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Module>>>;
        static auto ParseImpl(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Impl>>>;
        static auto ParseImplFunction(Context t_context, const SymbolName& t_selfTypeName) -> Expected<ParseData<std::shared_ptr<const Node::Function>>>;
        static auto ParseImplFunctionTemplate(Context t_context, const SymbolName& t_selfTypeName) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseTemplatedImpl(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::TemplatedImpl>>>;
        static auto ParseTemplatedImplFunction(Context t_context, const SymbolName& t_selfTypeName, const std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>>& t_implTemplateParameters) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseFunction(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Function>>>;
        static auto ParseFunctionTemplate(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>;
        static auto ParseParameters(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Variable::Parameter::Normal>>>>;
        static auto ParseVariable(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Variable::Normal::Static>>>;
        static auto ParseMemberVariable(Context t_context, const size_t& t_index) -> Expected<ParseData<std::shared_ptr<const Node::Variable::Normal::Instance>>>;
        static auto ParseType(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Type::IBase>>>;
        static auto ParseTypeTemplate(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Type>>>;
        static auto ParseStruct(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Type::Struct>>>;
        static auto ParseStructBody(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>>>>;
        static auto ParseStructTemplate(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Type>>>;
        static auto ParseStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::IBase>>>;
        static auto ParseExpressionStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Expression>>>;
        static auto ParseAssignmentStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assignment::Normal>>>;
        static auto ParseCompoundAssignmentStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assignment::Compound>>>;
        static auto ParseVariableStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Variable>>>;
        static auto ParseKeywordStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::IBase>>>;
        static auto ParseIfStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::If>>>;
        static auto ParseIfBlock(Context t_context) -> Expected<ParseData<std::pair<std::shared_ptr<const Node::Expression::IBase>, std::shared_ptr<const Node::Statement::Block>>>>;
        static auto ParseElifBlock(Context t_context) -> Expected<ParseData<std::pair<std::shared_ptr<const Node::Expression::IBase>, std::shared_ptr<const Node::Statement::Block>>>>;
        static auto ParseElseBlock(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Block>>>;
        static auto ParseWhileStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::While>>>;
        static auto ParseReturnStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Return>>>;
        static auto ParseExitStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Exit>>>;
        static auto ParseAssertStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assert>>>;
        static auto ParseBlockStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Block>>>;
        static auto ParseExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>;
        static auto ParseSimpleExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>;
        static auto ParseMemberAccessExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::MemberAccess>>>;
        static auto ParseArguments(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Expression::IBase>>>>;
        static auto ParsePrimaryExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>;
        static auto ParseExpressionExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Expression>>>;
        static auto ParseLiteralExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Literal>>>;
        static auto ParseLiteralSymbolExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::LiteralSymbol>>>;
        static auto ParseStructConstructionExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::StructConstruction>>>;
        static auto ParseCastExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Cast>>>;
        static auto ParseAddressOfExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::AddressOf>>>;
        static auto ParseSizeOfExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::SizeOf>>>;
        static auto ParseDerefAsExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::DerefAs>>>;
        static auto ParseAttribute(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Attribute>>>;
        static auto ParseAttributes(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Attribute>>>>;
    };
}
