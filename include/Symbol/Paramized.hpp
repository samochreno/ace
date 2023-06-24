#include "Symbol/Base.hpp"

#include <vector>

#include "Symbol/Var/Param/Base.hpp"

namespace Ace::Symbol
{
    class IParamized : public virtual Symbol::IBase
    {
    public:
        virtual ~IParamized() = default;

        virtual auto CollectParams()     const -> std::vector<Symbol::Var::Param::IBase*> = 0;
        virtual auto CollectParamTypes() const -> std::vector<Symbol::Type::IBase*> final;
    };
}
