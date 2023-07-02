#pragma once

#include "Nodes/AttributeNode.hpp"
#include "Nodes/Node.hpp"
#include "Nodes/Exprs/AndExprNode.hpp"
#include "Nodes/Exprs/AddressOfExprNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Exprs/UserBinaryExprNode.hpp"
#include "Nodes/Exprs/BoxExprNode.hpp"
#include "Nodes/Exprs/CastExprNode.hpp"
#include "Nodes/Exprs/DerefAsExprNode.hpp"
#include "Nodes/Exprs/ExprExprNode.hpp"
#include "Nodes/Exprs/FunctionCallExprNode.hpp"
#include "Nodes/Exprs/LiteralExprNode.hpp"
#include "Nodes/Exprs/LiteralSymbolExprNode.hpp"
#include "Nodes/Exprs/LogicalNegationExprNode.hpp"
#include "Nodes/Exprs/MemberAccessExprNode.hpp"
#include "Nodes/Exprs/OrExprNode.hpp"
#include "Nodes/Exprs/SizeOfExprNode.hpp"
#include "Nodes/Exprs/StructConstructionExprNode.hpp"
#include "Nodes/Exprs/UserUnaryExprNode.hpp"
#include "Nodes/Exprs/UnboxExprNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/ImplNode.hpp"
#include "Nodes/ModuleNode.hpp"
#include "Nodes/Stmts/AssertStmtNode.hpp"
#include "Nodes/Stmts/CompoundAssignmentStmtNode.hpp"
#include "Nodes/Stmts/AssignmentStmtNode.hpp"
#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "Nodes/Stmts/ExitStmtNode.hpp"
#include "Nodes/Stmts/ExprStmtNode.hpp"
#include "Nodes/Stmts/IfStmtNode.hpp"
#include "Nodes/Stmts/LabelStmtNode.hpp"
#include "Nodes/Stmts/ReturnStmtNode.hpp"
#include "Nodes/Stmts/VarStmtNode.hpp"
#include "Nodes/Stmts/WhileStmtNode.hpp"
#include "Nodes/Templates/TemplateNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "Nodes/Templates/TypeTemplateNode.hpp"
#include "Nodes/TemplatedImplNode.hpp"
#include "Nodes/TemplateParams/ImplTemplateParamNode.hpp"
#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"
#include "Nodes/Types/TypeNode.hpp"
#include "Nodes/Types/StructTypeNode.hpp"
#include "Nodes/TypedNode.hpp"
#include "Nodes/Vars/InstanceVarNode.hpp"
#include "Nodes/Vars/StaticVarNode.hpp"
#include "Nodes/Vars/Params/NormalParamVarNode.hpp"
#include "Nodes/Vars/Params/SelfParamVarNode.hpp"