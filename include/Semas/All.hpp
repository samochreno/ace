#pragma once

#include "Semas/Sema.hpp"
#include "Semas/Exprs/AddressOfExprSema.hpp"
#include "Semas/Exprs/AndExprSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Exprs/UserBinaryExprSema.hpp"
#include "Semas/Exprs/BoxExprSema.hpp"
#include "Semas/Exprs/ConversionPlaceholderExprSema.hpp"
#include "Semas/Exprs/DerefAsExprSema.hpp"
#include "Semas/Exprs/DerefExprSema.hpp"
#include "Semas/Exprs/ExprExprSema.hpp"
#include "Semas/Exprs/Calls/InstanceCallExprSema.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "Semas/Exprs/LiteralExprSema.hpp"
#include "Semas/Exprs/LogicalNegationExprSema.hpp"
#include "Semas/Exprs/OrExprSema.hpp"
#include "Semas/Exprs/RefExprSema.hpp"
#include "Semas/Exprs/SizeOfExprSema.hpp"
#include "Semas/Exprs/StructConstructionExprSema.hpp"
#include "Semas/Exprs/TypeInfoPtrExprSema.hpp"
#include "Semas/Exprs/UserUnaryExprSema.hpp"
#include "Semas/Exprs/UnboxExprSema.hpp"
#include "Semas/Exprs/VarRefs/FieldVarRefExprSema.hpp"
#include "Semas/Exprs/VarRefs/StaticVarRefExprSema.hpp"
#include "Semas/Exprs/VtblPtrExprSema.hpp"
#include "Semas/Stmts/AssertStmtSema.hpp"
#include "Semas/Stmts/Assignments/CompoundAssignmentStmtSema.hpp"
#include "Semas/Stmts/Assignments/SimpleAssignmentStmtSema.hpp"
#include "Semas/Stmts/BlockEndStmtSema.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"
#include "Semas/Stmts/CopyStmtSema.hpp"
#include "Semas/Stmts/DropStmtSema.hpp"
#include "Semas/Stmts/ExitStmtSema.hpp"
#include "Semas/Stmts/ExpandableStmtSema.hpp"
#include "Semas/Stmts/ExprStmtSema.hpp"
#include "Semas/Stmts/GroupStmtSema.hpp"
#include "Semas/Stmts/IfStmtSema.hpp"
#include "Semas/Stmts/Jumps/ConditionalJumpStmtSema.hpp"
#include "Semas/Stmts/Jumps/NormalJumpStmtSema.hpp"
#include "Semas/Stmts/LabelStmtSema.hpp"
#include "Semas/Stmts/RetStmtSema.hpp"
#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Stmts/VarStmtSema.hpp"
#include "Semas/Stmts/WhileStmtSema.hpp"
