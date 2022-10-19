#include <gtest/gtest.h>
#include <cstdint>
#include "cvi_signal_slot/cvi_signal_slot.h"

namespace {

struct TestContext
{
    int check;
};

struct TestData
{
    int check;
};

int test_handler_none(void *handle) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check++;
    return 0;
}

int test_handler_void(void *handle, void *arg1) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check++;
    TestData *arg1_data = (TestData *)arg1;
    EXPECT_EQ(arg1_data->check, 0);
    arg1_data->check++;
    return 0;
}

int test_handler2_void(void *handle, TestData *arg1) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check++;
    arg1->check++;
    return 0;
}

int test_handler_int(void *handle, int arg1) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check++;
    EXPECT_EQ(arg1, 1);
    return 0;
}

int test_handler2_int(void *handle, int arg1) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check++;
    EXPECT_EQ(arg1, 1);
    return 0;
}

int test_handler_int64(void *handle, int64_t arg1) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check++;
    EXPECT_EQ(arg1, 4000000000);
    return 0;
}

int test_handler_int_int(void *handle, int arg1, int arg2) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check++;
    EXPECT_EQ(arg1, 1);
    EXPECT_EQ(arg2, 2);
    return 0;
}

int test_handler_uint32_uint32(void *handle, uint32_t arg1, uint32_t arg2) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check++;
    EXPECT_EQ(arg1, 1);
    EXPECT_EQ(arg2, 2);
    return 0;
}

void test_void_handler_none(void *handle) {
    TestContext *context = static_cast<TestContext*>(handle);
    context->check += 2;
}

TEST(UnitTest, BaseTest) {
    TestContext context = {};
    TestContext *handle = &context;

    CVI_SLOT slot_none = {};
    CVI_SLOT_Init(&slot_none, handle, test_handler_none);

    CVI_SLOT slot_void = {};
    CVI_SLOT_Init(&slot_void, handle, test_handler_void);

    CVI_SLOT slot2_void = {};
    // cast to void type slot
    CVI_SLOT_Init(&slot2_void, handle, (CVI_INT_SLOT_VOID_HANLDER)test_handler2_void);

    CVI_SLOT slot_int = {};
    CVI_SLOT_Init(&slot_int, handle, test_handler_int);

    CVI_SLOT slot2_int = {};
    CVI_SLOT_Init(&slot2_int, handle, test_handler2_int);

    CVI_SLOT slot_int64 = {};
    CVI_SLOT_Init(&slot_int64, handle, test_handler_int64);

    CVI_SLOT slot_int_int = {};
    CVI_SLOT_Init(&slot_int_int, handle, test_handler_int_int);

    CVI_SLOT slot_uint32_uint32 = {};
    CVI_SLOT_Init(&slot_uint32_uint32, handle, test_handler_uint32_uint32);

    CVI_SIGNAL signal_none = {};
    signal_none.type = CVI_SIGNAL_SLOT_TYPE_NONE;
    CVI_SIGNAL_Init(&signal_none);

    CVI_SIGNAL signal_void = {};
    CVI_SIGNAL_InitByType(&signal_void, CVI_SIGNAL_SLOT_TYPE_VOID);

    CVI_SIGNAL signal_int = {};
    signal_int.type = CVI_SIGNAL_SLOT_TYPE_INT;
    CVI_SIGNAL_Init(&signal_int);

    CVI_SIGNAL signal_int64 = {};
    CVI_SIGNAL_InitByType(&signal_int64, CVI_SIGNAL_SLOT_TYPE_INT64);

    CVI_SIGNAL signal_int_int = {};
    CVI_SIGNAL_InitByType(&signal_int_int, CVI_SIGNAL_SLOT_TYPE_INT_INT);

    CVI_SIGNAL signal_uint32_uint32 = {};
    CVI_SIGNAL_InitByType(&signal_uint32_uint32, CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32);

    CVI_SIGNAL_Connect(signal_none, slot_none);
    context.check = 0;
    CVI_SIGNAL_Emit(signal_none);
    EXPECT_EQ(context.check, 1);

    CVI_SIGNAL_Connect(signal_void, slot_void);
    CVI_SIGNAL_Connect(signal_void, slot2_void);
    context.check = 0;
    TestData data = {};
    CVI_SIGNAL_Emit(signal_void, &data);
    EXPECT_EQ(context.check, 2);
    EXPECT_EQ(data.check, 2);
    CVI_SIGNAL_Disconnect(signal_void, slot_void);
    context.check = 0;
    data.check = 0;
    CVI_SIGNAL_Emit(signal_void, &data);
    EXPECT_EQ(context.check, 1);
    EXPECT_EQ(data.check, 1);

    CVI_SIGNAL_Connect(signal_int, slot_int);
    CVI_SIGNAL_Connect(signal_int, slot2_int);
    context.check = 0;
    CVI_SIGNAL_Emit(signal_int, 1);
    EXPECT_EQ(context.check, 2);

    CVI_SIGNAL_Connect(signal_int64, slot_int64);
    context.check = 0;
    CVI_SIGNAL_Emit(signal_int64, (int64_t)4000000000);
    EXPECT_EQ(context.check, 1);

    CVI_SIGNAL_Connect(signal_int_int, slot_int_int);
    context.check = 0;
    CVI_SIGNAL_Emit(signal_int_int, 1, 2);
    EXPECT_EQ(context.check, 1);

    CVI_SIGNAL_Connect(signal_uint32_uint32, slot_uint32_uint32);
    context.check = 0;
    CVI_SIGNAL_Emit(signal_uint32_uint32, (uint32_t)1, (uint32_t)2);
    EXPECT_EQ(context.check, 1);

    CVI_SIGNAL_Deinit(&signal_none);
    CVI_SIGNAL_Deinit(&signal_void);
    CVI_SIGNAL_Deinit(&signal_int);
    CVI_SIGNAL_Deinit(&signal_int64);
    CVI_SIGNAL_Deinit(&signal_int_int);
    CVI_SIGNAL_Deinit(&signal_uint32_uint32);
    CVI_SLOT_Deinit(&slot_none);
    CVI_SLOT_Deinit(&slot_void);
    CVI_SLOT_Deinit(&slot2_void);
    CVI_SLOT_Deinit(&slot_int);
    CVI_SLOT_Deinit(&slot2_int);
    CVI_SLOT_Deinit(&slot_int64);
    CVI_SLOT_Deinit(&slot_int_int);
    CVI_SLOT_Deinit(&slot_uint32_uint32);
}

TEST(UnitTest, VoidHandlerTest) {
    TestContext *handle = new TestContext();

    CVI_SLOT slot_none = {};
    // return int handler
    CVI_SLOT_Init(&slot_none, handle, test_handler_none);

    CVI_SLOT slot2_none = {};
    // return void handler
    CVI_SLOT_Init(&slot2_none, handle, test_void_handler_none);

    CVI_SIGNAL signal_none = {};
    signal_none.type = CVI_SIGNAL_SLOT_TYPE_NONE;
    CVI_SIGNAL_Init(&signal_none);

    CVI_SIGNAL_Connect(signal_none, slot_none);
    CVI_SIGNAL_Connect(signal_none, slot2_none);
    CVI_SIGNAL_Emit(signal_none);
    EXPECT_EQ(handle->check, 3);

    CVI_SIGNAL_Deinit(&signal_none);
    CVI_SLOT_Deinit(&slot_none);
    CVI_SLOT_Deinit(&slot2_none);

    delete static_cast<TestContext *>(handle);
    handle = nullptr;
}

} // anonymous namespace
