# CVI_SIGNAL_SLOT

CVI_SIGNAL and CVI_SLOT is a communicate mechanism between signal (event) and slot (handler).

Signal can connect to multiple slots, and when signal emit, all connected slots will be called.

## Prerequisites

* cvi_log

## How to use

Declare CVI_SIGNAL and call, the type is same as slot handler's type

```bash
CVI_SIGNAL_InitByType(CVI_SIGNAL *cvi_signal, CVI_SIGNAL_SLOT_TYPE type);
```

Current supported types are:

CVI_SIGNAL_SLOT_TYPE_NONE,
CVI_SIGNAL_SLOT_TYPE_VOID,
CVI_SIGNAL_SLOT_TYPE_BOOL,
CVI_SIGNAL_SLOT_TYPE_INT,
CVI_SIGNAL_SLOT_TYPE_INT64,
CVI_SIGNAL_SLOT_TYPE_FLOAT,
CVI_SIGNAL_SLOT_TYPE_STRING,
CVI_SIGNAL_SLOT_TYPE_INT_INT,
CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32

Declare CVI_SLOT and call

```bash
CVI_SLOT_Init(CVI_SLOT *cvi_slot, void *handle, CVI_SLOT_VOID_HANLDER handler);

int handler(void *handle, AnyType *data);
```

CVI_SLOT can init with any type pointer (one arg), just cast to CVI_SLOT_VOID_HANLDER.

```bash
# cast to void type handler, if handler is handler(void *handle, AnyType *data)
CVI_SLOT_Init(&slot, handle, (CVI_SLOT_VOID_HANLDER)handler);
CVI_SLOT_Init(&slot, handle, handler);
```

Connect signal to slot, the signal's type should be same as slot handler's type, or it will connect failed (return not 0).

```bash
CVI_SIGNAL_Connect(signal, slot);
```

Then emit signal, all connected slot will be called.

```bash
CVI_SIGNAL_Emit(signal, args...);
CVI_SIGNAL_Emit(signal);
CVI_SIGNAL_Emit(signal, 1);
CVI_SIGNAL_Emit(signal, (int64_t)4000000000);
CVI_SIGNAL_Emit(signal, 1, 2);
# type of args... is same as slot handler args type, and suggest declare type obviously.
# the value with args will pass to handler.
# CVI_SIGNAL_Emit(signal, (int64_t)1) with slot's handler(void *handle, int64_t arg1).
```

Can disconnect signal to slot, then this slot won't be called when signal emit.

```bash
CVI_SIGNAL_Disconnect(signal, slot);
```

After used, signal and slot need deinit.

```bash
CVI_SIGNAL_Deinit(&signal);
CVI_SLOT_Deinit(&slot);
```

## Limit

* Need check supported type, and signal's type should be same as slot's args type.

* Slot handler only support int and void return type.
