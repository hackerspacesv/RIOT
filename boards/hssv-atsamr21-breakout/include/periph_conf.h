#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include <stdint.h>

#include "cpu.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name   External oscillator and clock configuration
 *
 * There are three choices for selection of CORECLOCK:
 *
 * - usage of the 48 MHz DFLL fed by external oscillator running at 32 kHz
 * - usage of the PLL fed by the internal 8MHz oscillator divided by 8
 * - usage of the internal 8MHz oscillator directly, divided by N if needed
 *
 *
 * The PLL option allows for the usage of a wider frequency range and a more
 * stable clock with less jitter. This is why this option is default.
 *
 * The target frequency is computed from the PLL multiplier and the PLL divisor.
 * Use the following formula to compute your values:
 *
 * CORECLOCK = ((PLL_MUL + 1) * 1MHz) / PLL_DIV
 *
 * NOTE: The PLL circuit does not run with less than 32MHz while the maximum PLL
 *       frequency is 96MHz. So PLL_MULL must be between 31 and 95!
 *
 *
 * The internal Oscillator used directly can lead to a slightly better power
 * efficiency to the cost of a less stable clock. Use this option when you know
 * what you are doing! The actual core frequency is adjusted as follows:
 *
 * CORECLOCK = 8MHz / DIV
 *
 * NOTE: A core clock frequency below 1MHz is not recommended
 *
 * @{
 */
#define CLOCK_USE_PLL               (1)
#define CLOCK_USE_XOSC32_DFLL       (0)
/*
 * 0: use XOSC32K (always 32.768kHz) to clock GCLK2
 * 1: use OSCULP32K factory calibrated (~32.768kHz) to clock GCLK2
 *
 * OSCULP32K is factory calibrated to be around 32.768kHz but this values can
 * be of by a couple off % points, so prefer XOSC32K as default configuration.
 */
#define GEN2_ULP32K                 (0)

#if CLOCK_USE_PLL
/* edit these values to adjust the PLL output frequency */
#define CLOCK_PLL_MUL       (47U)               /* must be >= 31 & <= 95 */
#define CLOCK_PLL_DIV       (1U)                /* adjust to your needs */
/* generate the actual used core clock frequency */
#define CLOCK_CORECLOCK     (((CLOCK_PLL_MUL + 1) * 1000000U) / CLOCK_PLL_DIV)
#elif CLOCK_USE_XOSC32_DFLL
    /* Settings for 32 kHz external oscillator and 48 MHz DFLL */
#define CLOCK_CORECLOCK     (48000000U)
#define CLOCK_XOSC32K       (32768UL)
#define CLOCK_8MHZ          (1)
#else
/* edit this value to your needs */
#define CLOCK_DIV           (1U)
/* generate the actual core clock frequency */
#define CLOCK_CORECLOCK     (8000000 / CLOCK_DIV)
#endif
/** @} */

/**
 * @name    Timer peripheral configuration
 * @{
 */
static const tc32_conf_t timer_config[] = {
    {   /* Timer 0 - System Clock */
        .dev            = TC3,
        .irq            = TC3_IRQn,
        .pm_mask        = PM_APBCMASK_TC3,
        .gclk_ctrl      = GCLK_CLKCTRL_ID_TCC2_TC3,
#if CLOCK_USE_PLL || CLOCK_USE_XOSC32_DFLL
        .gclk_src       = SAM0_GCLK_1MHZ,
#else
        .gclk_src       = SAM0_GCLK_MAIN,
#endif
        .flags          = TC_CTRLA_MODE_COUNT16,
    },
    {   /* Timer 1 */
        .dev            = TC4,
        .irq            = TC4_IRQn,
        .pm_mask        = PM_APBCMASK_TC4 | PM_APBCMASK_TC5,
        .gclk_ctrl      = GCLK_CLKCTRL_ID_TC4_TC5,
#if CLOCK_USE_PLL || CLOCK_USE_XOSC32_DFLL
        .gclk_src       = SAM0_GCLK_1MHZ,
#else
        .gclk_src       = SAM0_GCLK_MAIN,
#endif
        .flags          = TC_CTRLA_MODE_COUNT32,
    }
};

#define TIMER_0_MAX_VALUE   0xffff

/* interrupt function name mapping */
#define TIMER_0_ISR         isr_tc3
#define TIMER_1_ISR         isr_tc4

#define TIMER_NUMOF         ARRAY_SIZE(timer_config)
/** @} */

/**
 * @name    UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
    {
        .dev      = &SERCOM2->USART,
        .rx_pin   = GPIO_PIN(PA,15),
        .tx_pin   = GPIO_PIN(PA,14),
#ifdef MODULE_PERIPH_UART_HW_FC
        .rts_pin  = GPIO_UNDEF,
        .cts_pin  = GPIO_UNDEF,
#endif
        .mux      = GPIO_MUX_C,
        .rx_pad   = UART_PAD_RX_3,
        .tx_pad   = UART_PAD_TX_2,
        .flags    = UART_FLAG_NONE,
        .gclk_src = SAM0_GCLK_MAIN,
    },
};

/* interrupt function name mapping */
#define UART_0_ISR          isr_sercom2

#define UART_NUMOF          ARRAY_SIZE(uart_config)
/** @} */

/**
 * @name    SPI configuration
 * @{
 */
static const spi_conf_t spi_config[] = {
    {
        .dev      = &SERCOM4->SPI,
        .miso_pin = GPIO_PIN(PC, 19),
        .mosi_pin = GPIO_PIN(PB, 30),
        .clk_pin  = GPIO_PIN(PC, 18),
        .miso_mux = GPIO_MUX_F,
        .mosi_mux = GPIO_MUX_F,
        .clk_mux  = GPIO_MUX_F,
        .miso_pad = SPI_PAD_MISO_0,
        .mosi_pad = SPI_PAD_MOSI_2_SCK_3,
        .gclk_src = SAM0_GCLK_MAIN,
    },
    {
        .dev      = &SERCOM1->SPI,
        .miso_pin = GPIO_PIN(PA, 18),
        .mosi_pin = GPIO_PIN(PA, 16),
        .clk_pin  = GPIO_PIN(PA, 19),
        .miso_mux = GPIO_MUX_C,
        .mosi_mux = GPIO_MUX_C,
        .clk_mux  = GPIO_MUX_C,
        .miso_pad = SPI_PAD_MISO_2,
        .mosi_pad = SPI_PAD_MOSI_0_SCK_3,
        .gclk_src = SAM0_GCLK_MAIN,
    }
};

#define SPI_NUMOF           ARRAY_SIZE(spi_config)
/** @} */

/**
 * @name    I2C configuration
 * @{
 */
static const i2c_conf_t i2c_config[] = {
    {
        .dev      = &(SERCOM3->I2CM),
        .speed    = I2C_SPEED_NORMAL,
        .scl_pin  = GPIO_PIN(PA, 23),
        .sda_pin  = GPIO_PIN(PA, 22),
        .mux      = GPIO_MUX_C,
        .gclk_src = SAM0_GCLK_MAIN,
        .flags    = I2C_FLAG_NONE
     }
};
#define I2C_NUMOF          ARRAY_SIZE(i2c_config)
/** @} */

/**
 * @name    RTC configuration
 * @{
 */
#define RTC_DEV             RTC->MODE2
/** @} */

/**
 * @name    RTT configuration
 * @{
 */
#define RTT_DEV             RTC->MODE0
#define RTT_IRQ             RTC_IRQn
#define RTT_IRQ_PRIO        10
#define RTT_ISR             isr_rtc
#define RTT_MAX_VALUE       (0xffffffff)
#define RTT_FREQUENCY       (32768U)    /* in Hz. For changes see `rtt.c` */
#define RTT_RUNSTDBY        (1)         /* Keep RTT running in sleep states */
/** @} */

/**
 * @name ADC Configuration
 * @{
 */

/* ADC Default values */
#define ADC_PRESCALER                       ADC_CTRLB_PRESCALER_DIV512

#define ADC_NEG_INPUT                       ADC_INPUTCTRL_MUXNEG_GND
#define ADC_GAIN_FACTOR_DEFAULT             ADC_INPUTCTRL_GAIN_DIV2         /* 0 to 3.3V */
#define ADC_REF_DEFAULT                     ADC_REFCTRL_REFSEL_INTVCC1_Val

static const adc_conf_chan_t adc_channels[] = {
    /* port, pin, muxpos */
    {GPIO_PIN(PA, 5), ADC_INPUTCTRL_MUXPOS_PIN5},
    {GPIO_PIN(PA, 6), ADC_INPUTCTRL_MUXPOS_PIN6},
};

#define ADC_NUMOF                           ARRAY_SIZE(adc_channels)
/** @} */

/**
 * @name USB peripheral configuration
 * @{
 */
static const sam0_common_usb_config_t sam_usbdev_config[] = {
    {
        .dm     = GPIO_PIN(PA, 24),
        .dp     = GPIO_PIN(PA, 25),
        .d_mux  = GPIO_MUX_G,
        .device = &USB->DEVICE,
        .gclk_src = SAM0_GCLK_MAIN,
    }
};
/** @} */
#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
