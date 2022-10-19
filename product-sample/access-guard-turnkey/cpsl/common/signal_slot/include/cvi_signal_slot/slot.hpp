#pragma once

#include <functional>

namespace cvi_signal_slot {

template <typename... Args>
class SlotInterface
{
public:
    virtual ~SlotInterface() = default;
    virtual void operator()(Args... args) = 0;
};

template <typename ReturnType, typename... Args>
class Slot : SlotInterface<Args...>
{
public:
    using Handle = void*;
    using Handler = std::function<ReturnType(Handle, Args...)>;

    Slot(Handle handle, const Handler &handler) :
    handle(handle),
    handler(handler)
    {}

    Slot(Handle handle, Handler &&handler) :
    handle(handle),
    handler(std::move(handler))
    {}

    virtual void operator()(Args... args) override
    {
        if (handler) {
            handler(handle, args...);
        }
    }

private:
    Handle handle{nullptr};
    Handler handler;
};

} // namespace cvi_signal_slot
