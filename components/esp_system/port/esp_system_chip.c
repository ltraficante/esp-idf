/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include "esp_cpu.h"
#include "soc/rtc.h"
#include "esp_private/panic_internal.h"
#include "esp_private/system_internal.h"
#include "esp_heap_caps.h"
#include "esp_rom_uart.h"
#include "esp_rom_sys.h"
#include "sdkconfig.h"

void IRAM_ATTR esp_restart_noos_dig(void)
{
    // make sure all the panic handler output is sent from UART FIFO
    if (CONFIG_ESP_CONSOLE_UART_NUM >= 0) {
        esp_rom_uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);
    }

    // switch to XTAL (otherwise we will keep running from the PLL)
    rtc_clk_cpu_freq_set_xtal();

    // esp_restart_noos_dig() will generates a core reset, which does not reset the
    // registers of the RTC domain, so the CPU's stall state remains after the reset,
    // we need to release them here
#if !CONFIG_FREERTOS_UNICORE
    // unstall all other cores
    int core_id = esp_cpu_get_core_id();
    for (uint32_t i = 0; i < SOC_CPU_CORES_NUM; i++) {
        if (i != core_id) {
            esp_cpu_unstall(i);
        }
    }
#endif
    // generate core reset
    esp_rom_software_reset_system();
    while (true) {
        ;
    }
}

uint32_t esp_get_free_heap_size( void )
{
    return heap_caps_get_free_size( MALLOC_CAP_DEFAULT );
}

uint32_t esp_get_free_internal_heap_size( void )
{
    return heap_caps_get_free_size( MALLOC_CAP_8BIT | MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL );
}

uint32_t esp_get_minimum_free_heap_size( void )
{
    return heap_caps_get_minimum_free_size( MALLOC_CAP_DEFAULT );
}

const char *esp_get_idf_version(void)
{
    return IDF_VER;
}

void __attribute__((noreturn)) esp_system_abort(const char *details)
{
    panic_abort(details);
}