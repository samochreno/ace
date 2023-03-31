#pragma once

#include "Node/Base.hpp"
#include "Node/Attribute.hpp"
#include "Node/Impl.hpp"
#include "Node/Module.hpp"
#include "Node/TemplatedImpl.hpp"

#include "Node/Expression/Base.hpp"
#include "Node/Expression/Cast.hpp"
#include "Node/Expression/Expression.hpp"
#include "Node/Expression/FunctionCall.hpp"
#include "Node/Expression/Literal.hpp"
#include "Node/Expression/LiteralSymbol.hpp"
#include "Node/Expression/MemberAccess.hpp"
#include "Node/Expression/SizeOf.hpp"
#include "Node/Expression/StructConstruction.hpp"

#include "Node/Expression/And.hpp"
#include "Node/Expression/Or.hpp"
#include "Node/Expression/BinaryUser.hpp"

#include "Node/Expression/Box.hpp"
#include "Node/Expression/AddressOf.hpp"
#include "Node/Expression/DerefAs.hpp"
#include "Node/Expression/LogicalNegation.hpp"
#include "Node/Expression/Unbox.hpp"
#include "Node/Expression/UnaryUser.hpp"

#include "Node/Statement/Base.hpp"
#include "Node/Statement/Assert.hpp"
#include "Node/Statement/Block.hpp"
#include "Node/Statement/Exit.hpp"
#include "Node/Statement/Expression.hpp"
#include "Node/Statement/If.hpp"
#include "Node/Statement/Label.hpp"
#include "Node/Statement/Return.hpp"
#include "Node/Statement/Variable.hpp"
#include "Node/Statement/While.hpp"

#include "Node/Statement/Assignment/Compound.hpp"
#include "Node/Statement/Assignment/Normal.hpp"

#include "Node/Template/Base.hpp"
#include "Node/Template/Function.hpp"
#include "Node/Template/Type.hpp"

#include "Node/Type/Base.hpp"
#include "Node/Type/Struct.hpp"

#include "Node/Typed.hpp"
#include "Node/Function.hpp"

#include "Node/Variable/Normal/Instance.hpp"
#include "Node/Variable/Normal/Static.hpp"

#include "Node/Variable/Parameter/Normal.hpp"
#include "Node/Variable/Parameter/Self.hpp"
