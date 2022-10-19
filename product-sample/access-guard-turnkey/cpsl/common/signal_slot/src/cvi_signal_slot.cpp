#include "cvi_signal_slot/cvi_signal_slot.h"
#include "cvi_log.h"
#include "cvi_signal_slot/signal.hpp"
#include "cvi_signal_slot/slot.hpp"
#include "cvi_signal_slot/types.hpp"

using namespace cvi_signal_slot;

namespace {

template <typename ReturnType, typename... Args>
inline void initSlot(CVI_SLOT *cvi_slot, void *handle, ReturnType(*handler)(void*, Args...))
{
    if (!cvi_slot) {
        CVI_LOGE("Slot is null");
        return;
    }

    cvi_slot->type = SignalSlotType<std::decay_t<Args>...>::value;
    cvi_slot->slot = new Slot<ReturnType, Args...>(handle, handler);
}

template <typename... Ts>
inline Signal<Ts...>* getSignalPointer(const CVI_SIGNAL &cvi_signal)
{
    return static_cast<Signal<Ts...>*>(cvi_signal.signal);
}

template <typename... Ts>
inline SlotInterface<Ts...>* getSlotInterfacePointer(const CVI_SLOT &cvi_slot)
{
    return static_cast<SlotInterface<Ts...>*>(cvi_slot.slot);
}

template <typename... Ts>
inline void connect(const CVI_SIGNAL &cvi_signal, const CVI_SLOT &cvi_slot) {
    getSignalPointer<Ts...>(cvi_signal)->connect(*getSlotInterfacePointer<Ts...>(cvi_slot));
}

template <typename... Ts>
inline void disconnect(const CVI_SIGNAL &cvi_signal, const CVI_SLOT &cvi_slot) {
    getSignalPointer<Ts...>(cvi_signal)->disconnect(*getSlotInterfacePointer<Ts...>(cvi_slot));
}

template <typename... Args>
inline void emitSignal(const CVI_SIGNAL &cvi_signal, Args&&... args) {
    if (!cvi_signal.signal) {
        CVI_LOGE("Signal is null");
        return;
    }

    CVI_SIGNAL_SLOT_TYPE args_type = SignalSlotType<std::decay_t<Args>...>::value;
    if (!isCompatibleType(cvi_signal.type, args_type)) {
        CVI_LOGW("Signal skip emit because args type incompatible, type is %d and should be %d",
            static_cast<int>(args_type), static_cast<int>(cvi_signal.type));
        return;
    }

    getSignalPointer<std::remove_reference_t<Args>...>(cvi_signal)->emit(std::forward<Args>(args)...);
}

template <typename... Ts>
inline void destroySignal(CVI_SIGNAL &cvi_signal)
{
    auto signal = getSignalPointer<Ts...>(cvi_signal);
    if (signal) {
        delete signal;
        cvi_signal.signal = nullptr;
    }
}

template <typename... Ts>
inline void destroySlot(CVI_SLOT &cvi_slot)
{
    auto slot = getSlotInterfacePointer<Ts...>(cvi_slot);
    if (slot) {
        delete slot;
        cvi_slot.slot = nullptr;
    }
}

} // anonymous namespace

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal)
{
    emitSignal(cvi_signal);
}

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, void *arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, bool arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, int arg1)
{
    if (cvi_signal.type == CVI_SIGNAL_SLOT_TYPE_INT64) {
        CVI_LOGW("Implicit cast from int64_t to int");
        emitSignal(cvi_signal, static_cast<int64_t>(arg1));
    } else {
        emitSignal(cvi_signal, arg1);
    }
}

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, int64_t arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, float arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, char *arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, int arg1, int arg2)
{
    emitSignal(cvi_signal, arg1, arg2);
}

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, uint32_t arg1, uint32_t arg2)
{
    emitSignal(cvi_signal, arg1, arg2);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_VOID_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_BOOL_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT64_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_FLOAT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_STRING_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT_INT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_UINT32_UINT32_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_VOID_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_BOOL_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT64_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_FLOAT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_STRING_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT_INT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_UINT32_UINT32_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SIGNAL_Init(CVI_SIGNAL *cvi_signal)
{
    if (!cvi_signal) {
        CVI_LOGE("Signal is null");
        return;
    }

    switch(cvi_signal->type) {
    case CVI_SIGNAL_SLOT_TYPE_VOID:
        cvi_signal->signal = new Signal<void*>();
        break;
    case CVI_SIGNAL_SLOT_TYPE_BOOL:
        cvi_signal->signal = new Signal<bool>();
        break;
    case CVI_SIGNAL_SLOT_TYPE_INT:
        cvi_signal->signal = new Signal<int>();
        break;
    case CVI_SIGNAL_SLOT_TYPE_INT64:
        cvi_signal->signal = new Signal<int64_t>();
        break;
    case CVI_SIGNAL_SLOT_TYPE_FLOAT:
        cvi_signal->signal = new Signal<float>();
        break;
    case CVI_SIGNAL_SLOT_TYPE_STRING:
        cvi_signal->signal = new Signal<char*>();
        break;
    case CVI_SIGNAL_SLOT_TYPE_INT_INT:
        cvi_signal->signal = new Signal<int, int>();
        break;
    case CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32:
        cvi_signal->signal = new Signal<uint32_t, uint32_t>();
        break;
    default:
        cvi_signal->signal = new Signal<>();
        break;
    }
}

void CVI_SIGNAL_InitByType(CVI_SIGNAL *cvi_signal, CVI_SIGNAL_SLOT_TYPE type)
{
    if (!cvi_signal) {
        CVI_LOGE("Signal is null");
        return;
    }

    cvi_signal->type = type;
    CVI_SIGNAL_Init(cvi_signal);
}

int CVI_SIGNAL_Connect(CVI_SIGNAL cvi_signal, CVI_SLOT cvi_slot)
{
    if (!cvi_signal.signal) {
        CVI_LOGE("Signal is null");
        return -1;
    }

    if (!cvi_slot.slot) {
        CVI_LOGE("Slot is null");
        return -1;
    }

    if (cvi_signal.type != cvi_slot.type) {
        CVI_LOGE("Signal type %d and slot type %d not match", cvi_signal.type, cvi_slot.type);
        return -1;
    }

    switch(cvi_signal.type) {
        case CVI_SIGNAL_SLOT_TYPE_VOID:
            connect<void*>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_BOOL:
            connect<bool>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT:
            connect<int>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT64:
            connect<int64_t>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_FLOAT:
            connect<float>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_STRING:
            connect<char*>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT_INT:
            connect<int, int>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32:
            connect<uint32_t, uint32_t>(cvi_signal, cvi_slot);
            break;
        default:
            connect<>(cvi_signal, cvi_slot);
            break;
    }

    return 0;
}

int CVI_SIGNAL_Disconnect(CVI_SIGNAL cvi_signal, CVI_SLOT cvi_slot)
{
    if (!cvi_signal.signal) {
        CVI_LOGE("Signal is null");
        return -1;
    }

    if (!cvi_slot.slot) {
        CVI_LOGE("Slot is null");
        return -1;
    }

    if (cvi_signal.type != cvi_slot.type) {
        CVI_LOGE("Signal type %d and slot type %d not match", cvi_signal.type, cvi_slot.type);
        return -1;
    }

    switch(cvi_signal.type) {
        case CVI_SIGNAL_SLOT_TYPE_VOID:
            disconnect<void*>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_BOOL:
            disconnect<bool>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT:
            disconnect<int>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT64:
            disconnect<int64_t>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_FLOAT:
            disconnect<float>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_STRING:
            disconnect<char*>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT_INT:
            disconnect<int, int>(cvi_signal, cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32:
            disconnect<uint32_t, uint32_t>(cvi_signal, cvi_slot);
            break;
        default:
            disconnect<>(cvi_signal, cvi_slot);
            break;
    }

    return 0;
}

void CVI_SIGNAL_NONE_Emit(CVI_SIGNAL cvi_signal)
{
    emitSignal(cvi_signal);
}

void CVI_SIGNAL_VOID_Emit(CVI_SIGNAL cvi_signal, void *arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_BOOL_Emit(CVI_SIGNAL cvi_signal, bool arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_INT_Emit(CVI_SIGNAL cvi_signal, int arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_INT64_Emit(CVI_SIGNAL cvi_signal, int64_t arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_FLOAT_Emit(CVI_SIGNAL cvi_signal, float arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_STRING_Emit(CVI_SIGNAL cvi_signal, char *arg1)
{
    emitSignal(cvi_signal, arg1);
}

void CVI_SIGNAL_INT_INT_Emit(CVI_SIGNAL cvi_signal, int arg1, int arg2)
{
    emitSignal(cvi_signal, arg1, arg2);
}

void CVI_SIGNAL_UINT32_UINT32_Emit(CVI_SIGNAL cvi_signal, uint32_t arg1, uint32_t arg2)
{
    emitSignal(cvi_signal, arg1, arg2);
}

void CVI_SIGNAL_Deinit(CVI_SIGNAL *cvi_signal)
{
    if (!cvi_signal) {
        CVI_LOGE("Signal is null");
        return;
    }

    switch(cvi_signal->type) {
        case CVI_SIGNAL_SLOT_TYPE_VOID:
            destroySignal<void*>(*cvi_signal);
            break;
        case CVI_SIGNAL_SLOT_TYPE_BOOL:
            destroySignal<bool>(*cvi_signal);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT:
            destroySignal<int>(*cvi_signal);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT64:
            destroySignal<int64_t>(*cvi_signal);
            break;
        case CVI_SIGNAL_SLOT_TYPE_FLOAT:
            destroySignal<float>(*cvi_signal);
            break;
        case CVI_SIGNAL_SLOT_TYPE_STRING:
            destroySignal<char*>(*cvi_signal);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT_INT:
            destroySignal<int, int>(*cvi_signal);
            break;
        case CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32:
            destroySignal<uint32_t, uint32_t>(*cvi_signal);
            break;
        default:
            destroySignal<>(*cvi_signal);
            break;
    }
}

void CVI_INT_SLOT_NONE_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_INT_SLOT_VOID_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_VOID_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_INT_SLOT_BOOL_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_BOOL_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_INT_SLOT_INT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_INT_SLOT_INT64_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT64_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_INT_SLOT_FLOAT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_FLOAT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_INT_SLOT_STRING_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_STRING_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_INT_SLOT_INT_INT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT_INT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_INT_SLOT_UINT32_UINT32_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_UINT32_UINT32_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_NONE_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_BOOL_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_BOOL_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_VOID_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_VOID_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_INT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_INT64_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT64_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_FLOAT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_FLOAT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_STRING_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_STRING_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_INT_INT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT_INT_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_VOID_SLOT_UINT32_UINT32_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_UINT32_UINT32_HANLDER handler)
{
    initSlot(cvi_slot, handle, handler);
}

void CVI_SLOT_Deinit(CVI_SLOT *cvi_slot)
{
    if (!cvi_slot) {
        CVI_LOGE("Slot is null");
        return;
    }

    switch(cvi_slot->type) {
        case CVI_SIGNAL_SLOT_TYPE_VOID:
            destroySlot<void*>(*cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_BOOL:
            destroySlot<bool>(*cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT:
            destroySlot<int>(*cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT64:
            destroySlot<int64_t>(*cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_FLOAT:
            destroySlot<float>(*cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_STRING:
            destroySlot<char*>(*cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_INT_INT:
            destroySlot<int, int>(*cvi_slot);
            break;
        case CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32:
            destroySlot<uint32_t, uint32_t>(*cvi_slot);
            break;
        default:
            destroySlot<>(*cvi_slot);
            break;
    }
}
