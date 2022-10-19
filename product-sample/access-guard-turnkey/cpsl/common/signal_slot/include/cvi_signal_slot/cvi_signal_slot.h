#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    CVI_SIGNAL_SLOT_TYPE_NONE,
    CVI_SIGNAL_SLOT_TYPE_VOID,
    CVI_SIGNAL_SLOT_TYPE_BOOL,
    CVI_SIGNAL_SLOT_TYPE_INT,
    CVI_SIGNAL_SLOT_TYPE_INT64,
    CVI_SIGNAL_SLOT_TYPE_FLOAT,
    CVI_SIGNAL_SLOT_TYPE_STRING,
    CVI_SIGNAL_SLOT_TYPE_INT_INT,
    CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32
} CVI_SIGNAL_SLOT_TYPE;

typedef struct
{
    CVI_SIGNAL_SLOT_TYPE type;
    void *signal;
} CVI_SIGNAL;

typedef struct
{
    CVI_SIGNAL_SLOT_TYPE type;
    void *slot;
} CVI_SLOT;

typedef int (*CVI_INT_SLOT_HANLDER)(void*);
typedef int (*CVI_INT_SLOT_VOID_HANLDER)(void*, void*);
typedef int (*CVI_INT_SLOT_BOOL_HANLDER)(void*, bool);
typedef int (*CVI_INT_SLOT_INT_HANLDER)(void*, int);
typedef int (*CVI_INT_SLOT_INT64_HANLDER)(void*, int64_t);
typedef int (*CVI_INT_SLOT_FLOAT_HANLDER)(void*, float);
typedef int (*CVI_INT_SLOT_STRING_HANLDER)(void*, char*);
typedef int (*CVI_INT_SLOT_INT_INT_HANLDER)(void*, int, int);
typedef int (*CVI_INT_SLOT_UINT32_UINT32_HANLDER)(void*, uint32_t, uint32_t);
typedef void (*CVI_VOID_SLOT_HANLDER)(void*);
typedef void (*CVI_VOID_SLOT_VOID_HANLDER)(void*, void*);
typedef void (*CVI_VOID_SLOT_BOOL_HANLDER)(void*, bool);
typedef void (*CVI_VOID_SLOT_INT_HANLDER)(void*, int);
typedef void (*CVI_VOID_SLOT_INT64_HANLDER)(void*, int64_t);
typedef void (*CVI_VOID_SLOT_FLOAT_HANLDER)(void*, float);
typedef void (*CVI_VOID_SLOT_STRING_HANLDER)(void*, char*);
typedef void (*CVI_VOID_SLOT_INT_INT_HANLDER)(void*, int, int);
typedef void (*CVI_VOID_SLOT_UINT32_UINT32_HANLDER)(void*, uint32_t, uint32_t);

#ifdef __cplusplus
    #define CVI_SIGNAL_Emit(...) CVI_SIGNAL_EmitCpp(__VA_ARGS__)
    #define CVI_SLOT_Init(...) CVI_SLOT_InitCpp(__VA_ARGS__)
#else
    #define CVI_ARG0_SIGNAL_Emit(SIGNAL) CVI_SIGNAL_NONE_Emit(SIGNAL)
    #define CVI_ARG1_SIGNAL_Emit(SIGNAL, ARG1) \
        _Generic((ARG1), \
            bool: CVI_SIGNAL_BOOL_Emit, \
            int: CVI_SIGNAL_INT_Emit, \
            int64_t: CVI_SIGNAL_INT64_Emit, \
            float: CVI_SIGNAL_FLOAT_Emit, \
            char*: CVI_SIGNAL_STRING_Emit, \
            default: CVI_SIGNAL_VOID_Emit \
        )(SIGNAL, ARG1)
    #define CVI_ARG2_SIGNAL_Emit(SIGNAL, ARG1, ARG2) \
        _Generic((ARG1), \
            int: _Generic((ARG2), \
                int: CVI_SIGNAL_INT_INT_Emit, \
                default: CVI_SIGNAL_INT_INT_Emit \
            ), \
            uint32_t: _Generic((ARG2), \
                uint32_t: CVI_SIGNAL_UINT32_UINT32_Emit, \
                default: CVI_SIGNAL_UINT32_UINT32_Emit \
            ) \
        )(SIGNAL, ARG1, ARG2)
    #define CVI_GET_MACRO(_1, _2, _3, NAME, ...) NAME
    #define CVI_SIGNAL_Emit(...) CVI_GET_MACRO(__VA_ARGS__, \
        CVI_ARG2_SIGNAL_Emit, \
        CVI_ARG1_SIGNAL_Emit, \
        CVI_ARG0_SIGNAL_Emit)(__VA_ARGS__)
    #define CVI_SLOT_Init(SLOT, HANLDE, HANDLER) \
        _Generic((HANDLER), \
            CVI_INT_SLOT_HANLDER: CVI_INT_SLOT_NONE_Init, \
            CVI_INT_SLOT_VOID_HANLDER: CVI_INT_SLOT_VOID_Init, \
            CVI_INT_SLOT_BOOL_HANLDER: CVI_INT_SLOT_BOOL_Init, \
            CVI_INT_SLOT_INT_HANLDER: CVI_INT_SLOT_INT_Init, \
            CVI_INT_SLOT_INT64_HANLDER: CVI_INT_SLOT_INT64_Init, \
            CVI_INT_SLOT_FLOAT_HANLDER: CVI_INT_SLOT_FLOAT_Init, \
            CVI_INT_SLOT_STRING_HANLDER: CVI_INT_SLOT_STRING_Init, \
            CVI_INT_SLOT_INT_INT_HANLDER: CVI_INT_SLOT_INT_INT_Init, \
            CVI_INT_SLOT_UINT32_UINT32_HANLDER: CVI_INT_SLOT_UINT32_UINT32_Init, \
            CVI_VOID_SLOT_HANLDER: CVI_VOID_SLOT_NONE_Init, \
            CVI_VOID_SLOT_VOID_HANLDER: CVI_VOID_SLOT_VOID_Init, \
            CVI_VOID_SLOT_BOOL_HANLDER: CVI_VOID_SLOT_BOOL_Init, \
            CVI_VOID_SLOT_INT_HANLDER: CVI_VOID_SLOT_INT_Init, \
            CVI_VOID_SLOT_INT64_HANLDER: CVI_VOID_SLOT_INT64_Init, \
            CVI_VOID_SLOT_FLOAT_HANLDER: CVI_VOID_SLOT_FLOAT_Init, \
            CVI_VOID_SLOT_STRING_HANLDER: CVI_VOID_SLOT_STRING_Init, \
            CVI_VOID_SLOT_INT_INT_HANLDER: CVI_VOID_SLOT_INT_INT_Init, \
            CVI_VOID_SLOT_UINT32_UINT32_HANLDER: CVI_VOID_SLOT_UINT32_UINT32_Init, \
            default: CVI_INT_SLOT_NONE_Init \
        )(SLOT, HANLDE, HANDLER)
#endif

#ifdef __cplusplus

void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal);
void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, void *arg1);
void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, bool arg1);
void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, int arg1);
void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, int64_t arg1);
void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, float arg1);
void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, char *arg1);
void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, int arg1, int arg2);
void CVI_SIGNAL_EmitCpp(CVI_SIGNAL cvi_signal, uint32_t arg1, uint32_t arg2);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_VOID_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_BOOL_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT64_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_FLOAT_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_STRING_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT_INT_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_UINT32_UINT32_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_VOID_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_BOOL_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT64_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_FLOAT_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_STRING_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT_INT_HANLDER handler);
void CVI_SLOT_InitCpp(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_UINT32_UINT32_HANLDER handler);

#endif

#ifdef __cplusplus
extern "C" {
#endif

void CVI_SIGNAL_Init(CVI_SIGNAL *cvi_signal);
void CVI_SIGNAL_InitByType(CVI_SIGNAL *cvi_signal, CVI_SIGNAL_SLOT_TYPE type);
int CVI_SIGNAL_Connect(CVI_SIGNAL cvi_signal, CVI_SLOT cvi_slot);
int CVI_SIGNAL_Disconnect(CVI_SIGNAL cvi_signal, CVI_SLOT cvi_slot);
void CVI_SIGNAL_NONE_Emit(CVI_SIGNAL cvi_signal);
void CVI_SIGNAL_VOID_Emit(CVI_SIGNAL cvi_signal, void *arg1);
void CVI_SIGNAL_BOOL_Emit(CVI_SIGNAL cvi_signal, bool arg1);
void CVI_SIGNAL_INT_Emit(CVI_SIGNAL cvi_signal, int arg1);
void CVI_SIGNAL_INT64_Emit(CVI_SIGNAL cvi_signal, int64_t arg1);
void CVI_SIGNAL_FLOAT_Emit(CVI_SIGNAL cvi_signal, float arg1);
void CVI_SIGNAL_STRING_Emit(CVI_SIGNAL cvi_signal, char *arg1);
void CVI_SIGNAL_INT_INT_Emit(CVI_SIGNAL cvi_signal, int arg1, int arg2);
void CVI_SIGNAL_UINT32_UINT32_Emit(CVI_SIGNAL cvi_signal, uint32_t arg1, uint32_t arg2);
void CVI_SIGNAL_Deinit(CVI_SIGNAL *cvi_signal);
void CVI_INT_SLOT_NONE_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_HANLDER handler);
void CVI_INT_SLOT_VOID_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_VOID_HANLDER handler);
void CVI_INT_SLOT_BOOL_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_BOOL_HANLDER handler);
void CVI_INT_SLOT_INT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT_HANLDER handler);
void CVI_INT_SLOT_INT64_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT64_HANLDER handler);
void CVI_INT_SLOT_FLOAT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_FLOAT_HANLDER handler);
void CVI_INT_SLOT_STRING_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_STRING_HANLDER handler);
void CVI_INT_SLOT_INT_INT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_INT_INT_HANLDER handler);
void CVI_INT_SLOT_UINT32_UINT32_Init(CVI_SLOT *cvi_slot, void *handle, CVI_INT_SLOT_UINT32_UINT32_HANLDER handler);
void CVI_VOID_SLOT_NONE_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_HANLDER handler);
void CVI_VOID_SLOT_BOOL_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_BOOL_HANLDER handler);
void CVI_VOID_SLOT_VOID_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_VOID_HANLDER handler);
void CVI_VOID_SLOT_INT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT_HANLDER handler);
void CVI_VOID_SLOT_INT64_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT64_HANLDER handler);
void CVI_VOID_SLOT_FLOAT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_FLOAT_HANLDER handler);
void CVI_VOID_SLOT_STRING_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_STRING_HANLDER handler);
void CVI_VOID_SLOT_INT_INT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_INT_INT_HANLDER handler);
void CVI_VOID_SLOT_UINT32_UINT32_Init(CVI_SLOT *cvi_slot, void *handle, CVI_VOID_SLOT_UINT32_UINT32_HANLDER handler);
void CVI_SLOT_Deinit(CVI_SLOT *cvi_slot);

#ifdef __cplusplus
}
#endif
