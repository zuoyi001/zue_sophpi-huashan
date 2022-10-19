#pragma once

#include <algorithm>
#include <vector>
#include "slot.hpp"

namespace cvi_signal_slot {

template <typename... Args>
class Signal
{
public:
    using SlotType = SlotInterface<Args...>;

    template <typename... Ts>
    using compareable_types = std::tuple<std::decay_t<Ts>...>;

    void connect(SlotType &slot) noexcept
    {
        connected_slots.push_back(&slot);
    }

    void disconnect(const SlotType &slot) noexcept
    {
        connected_slots.erase(
            std::remove_if(
                connected_slots.begin(),
                connected_slots.end(),
                [&slot] (const SlotType *connected_slot) {
                    return connected_slot == (&slot);
                }
            ),
            connected_slots.end()
        );
    }

    void disconnect() noexcept
    {
        connected_slots.clear();
    }

    template <typename... EmitArgs>
    void emit(EmitArgs&&... args) {
        static_assert(
            std::is_same<compareable_types<EmitArgs...>, compareable_types<Args...>>::value,
            "Emit args type not match"
        );

        for (auto slot : connected_slots) {
            (*slot)(std::forward<EmitArgs>(args)...);
        }
    }

private:
    std::vector<SlotType*> connected_slots;
};

} // namespace cvi_signal_slot
