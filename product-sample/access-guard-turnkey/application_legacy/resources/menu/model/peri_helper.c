#include "peri_helper.h"
#include "cvi_hal_gpio.h"
#include "app_peripheral.h"

#ifdef ENABLE_PERI

static void peri_gpio_set(PERIPHERAL_GPIO_NUM_E gpio_num, bool on);

void peri_helper_relay(bool on)
{
    peri_gpio_set(RELAY_GPIO, on);
}

void peri_helper_door_key(bool on)
{
    peri_gpio_set(DOORKEY_GPIO, on);
}

static void peri_gpio_set(PERIPHERAL_GPIO_NUM_E gpio_num, bool on)
{
    if (on)
    {
        CVI_HAL_GpioSetValue(gpio_num, CVI_HAL_GPIO_VALUE_H);
    }
    else
    {
        CVI_HAL_GpioSetValue(gpio_num, CVI_HAL_GPIO_VALUE_L);
    }
}

#endif