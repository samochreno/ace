#pragma once

namespace Ace
{
    class Emitter;
}

namespace Ace
{
    template<typename T>
    class IEmittable
    {
    public:
        virtual ~IEmittable() = default;

        virtual auto Emit(Emitter& t_emitter) const -> T = 0;
    };
}