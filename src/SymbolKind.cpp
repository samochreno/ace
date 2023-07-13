#include "SymbolKind.hpp"

#include <cstdint>
#include <unordered_map>

#include "Assert.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    static const std::unordered_map<SymbolKind, int8_t> CreationOrderMap
    {
        { SymbolKind::Module,            0 },
        { SymbolKind::Struct,            0 },
        { SymbolKind::TypeTemplate,      0 },
        { SymbolKind::Label,             0 },
        { SymbolKind::ImplTemplateParam, 0 },
        { SymbolKind::TemplateParam,     0 },
        { SymbolKind::TypeAlias,         1 },
        { SymbolKind::TemplatedImpl,     1 },
        { SymbolKind::Function,          3 },
        { SymbolKind::FunctionTemplate,  3 },
        { SymbolKind::StaticVar,         3 },
        { SymbolKind::InstanceVar,       3 },
        { SymbolKind::ParamVar,          2 },
        { SymbolKind::LocalVar,          3 },
    };

    constexpr auto operator&(
        const SymbolKind t_lhs,
        const SymbolKind t_rhs
    ) -> bool
    {
        return
            (static_cast<uint16_t>(t_lhs) & static_cast<uint16_t>(t_rhs)) != 0;
    }

    auto GetSymbolCreationOrder(const SymbolKind t_kind) -> int8_t
    {
        auto it = CreationOrderMap.find(t_kind);
        ACE_ASSERT(it != end(CreationOrderMap));
        return it->second;
    }
}
