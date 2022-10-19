#pragma once

#include "cvi_signal_slot.h"

namespace cvi_signal_slot {

inline bool isCompatibleType(CVI_SIGNAL_SLOT_TYPE source_type, CVI_SIGNAL_SLOT_TYPE target_type)
{
    bool compatible = false;
    switch(source_type) {
    case CVI_SIGNAL_SLOT_TYPE_BOOL:
    case CVI_SIGNAL_SLOT_TYPE_INT:
    case CVI_SIGNAL_SLOT_TYPE_INT64:
        compatible = (target_type == CVI_SIGNAL_SLOT_TYPE_BOOL) ||
                     (target_type == CVI_SIGNAL_SLOT_TYPE_INT) ||
                     (target_type == CVI_SIGNAL_SLOT_TYPE_INT64);
        break;
    case CVI_SIGNAL_SLOT_TYPE_INT_INT:
    case CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32:
        compatible = (target_type == CVI_SIGNAL_SLOT_TYPE_INT_INT) ||
                     (target_type == CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32);
        break;
    default:
        compatible = (source_type == target_type);
        break;
    }

    return compatible;
}

template <typename... Ts>
struct SignalSlotType;

template <>
struct SignalSlotType<>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_NONE;
};

template <>
struct SignalSlotType<void*>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_VOID;
};

template<>
struct SignalSlotType<bool>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_BOOL;
};

template <>
struct SignalSlotType<int>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_INT;
};

template <>
struct SignalSlotType<int64_t>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_INT64;
};

template <>
struct SignalSlotType<float>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_FLOAT;
};

template <>
struct SignalSlotType<char*>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_STRING;
};

template <>
struct SignalSlotType<int, int>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_INT_INT;
};

template <>
struct SignalSlotType<uint32_t, uint32_t>
{
    static constexpr CVI_SIGNAL_SLOT_TYPE value = CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32;
};

} // namespace cvi_signal_slot
