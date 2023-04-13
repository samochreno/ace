#include "SymbolKind.hpp"

#include <cstdint>

#include "Asserts.hpp"
#include "Error.hpp"

namespace Ace
{
    static const std::unordered_map<SymbolKind, int8_t> CreationOrderMap
    {
        { SymbolKind::None,                 0 },
        { SymbolKind::Module,               0 },
        { SymbolKind::Struct,               0 },
        { SymbolKind::TypeTemplate,         0 },
        { SymbolKind::Label,                0 },
        { SymbolKind::TypeAlias,            1 },
        { SymbolKind::TemplatedImpl,        1 },
        { SymbolKind::Function,             3 },
        { SymbolKind::FunctionTemplate,     3 },
        { SymbolKind::StaticVariable,       3 },
        { SymbolKind::InstanceVariable,     3 },
        { SymbolKind::ParameterVariable,    2 },
        { SymbolKind::LocalVariable,        3 },
    };

    auto GetSymbolCreationOrder(const SymbolKind& t_kind) -> int8_t
    {
        auto it = CreationOrderMap.find(t_kind);
        ACE_ASSERT(it != end(CreationOrderMap));
        return it->second;
    }
}
