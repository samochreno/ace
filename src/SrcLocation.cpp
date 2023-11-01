#include "SrcLocation.hpp"

#include <string_view>

#include "Compilation.hpp"

namespace Ace
{
    SrcLocation::SrcLocation(
        Compilation* const compilation
    ) : Buffer{ compilation->GetCLIArgBuffer() },
        CharacterBeginIterator{ begin(std::string_view{ compilation->GetCLIArgBuffer()->GetBuffer() }) },
        CharacterEndIterator  { end  (std::string_view{ compilation->GetCLIArgBuffer()->GetBuffer() }) }
    {
    }
}
