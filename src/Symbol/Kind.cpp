#include "Symbol/Kind.hpp"

#include <cstdint>

#include "Asserts.hpp"
#include "Error.hpp"

namespace Ace::Symbol
{
    static const std::unordered_map<Kind, int8_t> CreationOrderMap
    {
        { Kind::None,               0 },
        { Kind::Module,             0 },
        { Kind::Struct,             0 },
        { Kind::TypeTemplate,       0 },
        { Kind::Label,              0 },
        { Kind::TypeAlias,          1 },
        { Kind::TemplatedImpl,      1 },
        { Kind::Function,           3 },
        { Kind::FunctionTemplate,   3 },
        { Kind::StaticVariable,     3 },
        { Kind::InstanceVariable,   3 },
        { Kind::ParameterVariable,  2 },
        { Kind::LocalVariable,      3 },
    };

    auto GetCreationOrder(const Kind& t_kind) -> int8_t
    {
        auto it = CreationOrderMap.find(t_kind);
        ACE_ASSERT(it != end(CreationOrderMap));
        return it->second;
    }
}
