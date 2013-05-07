/************************************************************************************
 * arch/arm/src/kl/kl_internal.h
 *
 *   Copyright (C) 2013 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************************/

#ifndef __ARCH_ARM_SRC_KL_KL_INTERNAL_H
#define __ARCH_ARM_SRC_KL_KL_INTERNAL_H

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include <nuttx/irq.h>

#include "up_internal.h"
#include "kl_config.h"
#include "chip.h"
#include "chip/kl_port.h"

/************************************************************************************
 * Definitions
 ************************************************************************************/

/* Configuration ********************************************************************/

/* Bit-encoded input to kl_pinconfig() *****************************************/
/* General form (32-bits, only 22 bits are unused in the encoding):
 *
 * oooo mmmv iiii ifd- ---- -ppp ---b bbbb
 */

/* Bits 25-31: 7 bits are used to encode the basic pin configuration:
 *
 * oooo mmm- ---- ---- ---- ---- ---- ----
 * oooommm:
 * |   `--- mmm: mode
 * `------- oooo: options (may be combined)
 */

#define _PIN_MODE_SHIFT        (25) /* Bits 25-27: Pin mode */
#define _PIN_MODE_MASK         (7 << _PIN_MODE_SHIFT)
#define _PIN_OPTIONS_SHIFT     (28) /* Bits 28-31: Pin mode options */
#define _PIN_OPTIONS_MASK      (15 << _PIN_OPTIONS_SHIFT)

/* Port Modes */

#define _PIN_MODE_ANALOG       (0 << _PIN_MODE_SHIFT)  /* 000 Pin Disabled (Analog) */
#define _PIN_MODE_GPIO         (1 << _PIN_MODE_SHIFT)  /* 001 Alternative 1 (GPIO) */
#define _PIN_MODE_ALT2         (2 << _PIN_MODE_SHIFT)  /* 010 Alternative 2 */
#define _PIN_MODE_ALT3         (3 << _PIN_MODE_SHIFT)  /* 011 Alternative 3 */
#define _PIN_MODE_ALT4         (4 << _PIN_MODE_SHIFT)  /* 100 Alternative 4 */
#define _PIN_MODE_ALT5         (5 << _PIN_MODE_SHIFT)  /* 101 Alternative 5 */
#define _PIN_MODE_ALT6         (6 << _PIN_MODE_SHIFT)  /* 110 Alternative 6 */
#define _PIN_MODE_ALT7         (7 << _PIN_MODE_SHIFT)  /* 111 Alternative 7 */

/* Options for all digital modes (Alternatives 1-7).  None of the digital
 * options apply if the analog mode is selected.
 */

#define _PIN_IO_MASK           (1 << _PIN_OPTIONS_SHIFT) /* xxx1 Digital input/output mask */
#define _PIN_INPUT             (0 << _PIN_OPTIONS_SHIFT) /* xxx0 Digital input */
#define _PIN_OUTPUT            (1 << _PIN_OPTIONS_SHIFT) /* xxx1 Digital output */

#define _PIN_INPUT_PULLMASK    (7 << _PIN_OPTIONS_SHIFT) /* x111 Mask for pull-up or -down bits */
#define _PIN_INPUT_PULLDOWN    (2 << _PIN_OPTIONS_SHIFT) /* x010 Input with internal pull-down resistor */
#define _PIN_INPUT_PULLUP      (6 << _PIN_OPTIONS_SHIFT) /* x110 Input with internal pull-up resistor */

#define _PIN_OUTPUT_SLEW_MASK  (3 << _PIN_OPTIONS_SHIFT) /* xx11 Mask to test for slow slew rate */
#define _PIN_OUTPUT_FAST       (1 << _PIN_OPTIONS_SHIFT) /* xx01 Output with fast slew rate */
#define _PIN_OUTPUT_SLOW       (3 << _PIN_OPTIONS_SHIFT) /* xx11 Output with slow slew rate */
#define _PIN_OUTPUT_OD_MASK    (5 << _PIN_OPTIONS_SHIFT) /* x1x1 Mask to test for open drain */
#define _PIN_OUTPUT_OPENDRAIN  (5 << _PIN_OPTIONS_SHIFT) /* x1x1 Output with open drain enabled */
#define _PIN_OUTPUT_DRIVE_MASK (9 << _PIN_OPTIONS_SHIFT) /* 1xx1 Mask to test for high drive strengh */
#define _PIN_OUTPUT_LOWDRIVE   (1 << _PIN_OPTIONS_SHIFT) /* 0xx1 Output with low drive strength */
#define _PIN_OUTPUT_HIGHDRIVE  (9 << _PIN_OPTIONS_SHIFT) /* 1xx1 Output with high drive strength */

/* End-user pin modes and configurations.  Notes:  (1) None of the digital options
 * are available for the analog mode, (2) digital settings may be combined (OR'ed)
 * provided that input-only and output-only options are not intermixed.
 */

#define PIN_ANALOG             _PIN_MODE_ANALOG

#define GPIO_INPUT             (_PIN_MODE_GPIO | _PIN_INPUT)
#define GPIO_PULLDOWN          (_PIN_MODE_GPIO | _PIN_INPUT_PULLDOWN)
#define GPIO_PULLUP            (_PIN_MODE_GPIO | _PIN_INPUT_PULLUP)
#define GPIO_OUTPUT            (_PIN_MODE_GPIO | _PIN_OUTPUT)
#define GPIO_FAST              (_PIN_MODE_GPIO | _PIN_OUTPUT_FAST)
#define GPIO_SLOW              (_PIN_MODE_GPIO | _PIN_OUTPUT_SLOW)
#define GPIO_OPENDRAIN         (_PIN_MODE_GPIO | _PIN_OUTPUT_LOWDRIVE)
#define GPIO_LOWDRIVE          (_PIN_MODE_GPIO | _PIN_OUTPUT_OPENDRAIN)
#define GPIO_HIGHDRIVE         (_PIN_MODE_GPIO | _PIN_OUTPUT_HIGHDRIVE)

#define PIN_ALT2               _PIN_MODE_ALT2
#define PIN_ALT2_INPUT         (_PIN_MODE_ALT2 | _PIN_INPUT)
#define PIN_ALT2_PULLDOWN      (_PIN_MODE_ALT2 | _PIN_INPUT_PULLDOWN)
#define PIN_ALT2_PULLUP        (_PIN_MODE_ALT2 | _PIN_INPUT_PULLUP)
#define PIN_ALT2_OUTPUT        (_PIN_MODE_ALT2 | _PIN_OUTPUT)
#define PIN_ALT2_FAST          (_PIN_MODE_ALT2 | _PIN_OUTPUT_FAST)
#define PIN_ALT2_SLOW          (_PIN_MODE_ALT2 | _PIN_OUTPUT_SLOW)
#define PIN_ALT2_OPENDRAIN     (_PIN_MODE_ALT2 | _PIN_OUTPUT_LOWDRIVE)
#define PIN_ALT2_LOWDRIVE      (_PIN_MODE_ALT2 | _PIN_OUTPUT_OPENDRAIN)
#define PIN_ALT2_HIGHDRIVE     (_PIN_MODE_ALT2 | _PIN_OUTPUT_HIGHDRIVE)

#define PIN_ALT3               _PIN_MODE_ALT3
#define PIN_ALT3_INPUT         (_PIN_MODE_ALT3 | _PIN_INPUT)
#define PIN_ALT3_PULLDOWN      (_PIN_MODE_ALT3 | _PIN_INPUT_PULLDOWN)
#define PIN_ALT3_PULLUP        (_PIN_MODE_ALT3 | _PIN_INPUT_PULLUP)
#define PIN_ALT3_OUTPUT        (_PIN_MODE_ALT3 | _PIN_OUTPUT)
#define PIN_ALT3_FAST          (_PIN_MODE_ALT3 | _PIN_OUTPUT_FAST)
#define PIN_ALT3_SLOW          (_PIN_MODE_ALT3 | _PIN_OUTPUT_SLOW)
#define PIN_ALT3_OPENDRAIN     (_PIN_MODE_ALT3 | _PIN_OUTPUT_LOWDRIVE)
#define PIN_ALT3_LOWDRIVE      (_PIN_MODE_ALT3 | _PIN_OUTPUT_OPENDRAIN)
#define PIN_ALT3_HIGHDRIVE     (_PIN_MODE_ALT3 | _PIN_OUTPUT_HIGHDRIVE)

#define PIN_ALT4               _PIN_MODE_ALT4
#define PIN_ALT4_INPUT         (_PIN_MODE_ALT4 | _PIN_INPUT)
#define PIN_ALT4_PULLDOWN      (_PIN_MODE_ALT4 | _PIN_INPUT_PULLDOWN)
#define PIN_ALT4_PULLUP        (_PIN_MODE_ALT4 | _PIN_INPUT_PULLUP)
#define PIN_ALT4_OUTPUT        (_PIN_MODE_ALT4 | _PIN_OUTPUT)
#define PIN_ALT4_FAST          (_PIN_MODE_ALT4 | _PIN_OUTPUT_FAST)
#define PIN_ALT4_SLOW          (_PIN_MODE_ALT4 | _PIN_OUTPUT_SLOW)
#define PIN_ALT4_OPENDRAIN     (_PIN_MODE_ALT4 | _PIN_OUTPUT_LOWDRIVE)
#define PIN_ALT4_LOWDRIVE      (_PIN_MODE_ALT4 | _PIN_OUTPUT_OPENDRAIN)
#define PIN_ALT4_HIGHDRIVE     (_PIN_MODE_ALT4 | _PIN_OUTPUT_HIGHDRIVE)

#define PIN_ALT5               _PIN_MODE_ALT5
#define PIN_ALT5_INPUT         (_PIN_MODE_ALT5 | _PIN_INPUT)
#define PIN_ALT5_PULLDOWN      (_PIN_MODE_ALT5 | _PIN_INPUT_PULLDOWN)
#define PIN_ALT5_PULLUP        (_PIN_MODE_ALT5 | _PIN_INPUT_PULLUP)
#define PIN_ALT5_OUTPUT        (_PIN_MODE_ALT5 | _PIN_OUTPUT)
#define PIN_ALT5_FAST          (_PIN_MODE_ALT5 | _PIN_OUTPUT_FAST)
#define PIN_ALT5_SLOW          (_PIN_MODE_ALT5 | _PIN_OUTPUT_SLOW)
#define PIN_ALT5_OPENDRAIN     (_PIN_MODE_ALT5 | _PIN_OUTPUT_LOWDRIVE)
#define PIN_ALT5_LOWDRIVE      (_PIN_MODE_ALT5 | _PIN_OUTPUT_OPENDRAIN)
#define PIN_ALT5_HIGHDRIVE     (_PIN_MODE_ALT5 | _PIN_OUTPUT_HIGHDRIVE)

#define PIN_ALT6               _PIN_MODE_ALT6
#define PIN_ALT6_INPUT         (_PIN_MODE_ALT6 | _PIN_INPUT)
#define PIN_ALT6_PULLDOWN      (_PIN_MODE_ALT6 | _PIN_INPUT_PULLDOWN)
#define PIN_ALT6_PULLUP        (_PIN_MODE_ALT6 | _PIN_INPUT_PULLUP)
#define PIN_ALT6_OUTPUT        (_PIN_MODE_ALT6 | _PIN_OUTPUT)
#define PIN_ALT6_FAST          (_PIN_MODE_ALT6 | _PIN_OUTPUT_FAST)
#define PIN_ALT6_SLOW          (_PIN_MODE_ALT6 | _PIN_OUTPUT_SLOW)
#define PIN_ALT6_OPENDRAIN     (_PIN_MODE_ALT6 | _PIN_OUTPUT_LOWDRIVE)
#define PIN_ALT6_LOWDRIVE      (_PIN_MODE_ALT6 | _PIN_OUTPUT_OPENDRAIN)
#define PIN_ALT6_HIGHDRIVE     (_PIN_MODE_ALT6 | _PIN_OUTPUT_HIGHDRIVE)

#define PIN_ALT7               _PIN_MODE_ALT7
#define PIN_ALT7_INPUT         (_PIN_MODE_ALT7 | _PIN_INPUT)
#define PIN_ALT7_PULLDOWN      (_PIN_MODE_ALT7 | _PIN_INPUT_PULLDOWN)
#define PIN_ALT7_PULLUP        (_PIN_MODE_ALT7 | _PIN_INPUT_PULLUP)
#define PIN_ALT7_OUTPUT        (_PIN_MODE_ALT7 | _PIN_OUTPUT)
#define PIN_ALT7_FAST          (_PIN_MODE_ALT7 | _PIN_OUTPUT_FAST)
#define PIN_ALT7_SLOW          (_PIN_MODE_ALT7 | _PIN_OUTPUT_SLOW)
#define PIN_ALT7_OPENDRAIN     (_PIN_MODE_ALT7 | _PIN_OUTPUT_LOWDRIVE)
#define PIN_ALT7_LOWDRIVE      (_PIN_MODE_ALT7 | _PIN_OUTPUT_OPENDRAIN)
#define PIN_ALT7_HIGHDRIVE     (_PIN_MODE_ALT7 | _PIN_OUTPUT_HIGHDRIVE)

/* The initial value for GPIO (Alternative 1 outputs):
 *
 * ---- ---v ---- ---- ---- ---- ---- ----
 *
 * Passive Filter and digital filter enable are valid in all digital pin
 * muxing modes.
 */

#define GPIO_OUTPUT_ONE        (1 << 24)  /* Bit 24: 1:Initial output value=1 */
#define GPIO_OUTPUT_ZER0       (0)        /* Bit 24: 0:Initial output value=0 */

/* Five bits are used to incode DMA/interrupt options:
 *
 * ---- ---- iiii i--- ---- ---- ---- ----
 *
 * The pin interrupt configuration is valid in all digital pin muxing modes
 * (restricted to inputs).
 */

#define _PIN_INT_SHIFT         (20)
#define _PIN_INT_MASK          (31 << _PIN_INT_SHIFT)

#define _PIN_INTDMA_MASK       (3 << _PIN_INT_SHIFT)
#define _PIN_INTDMA_NONE       (0 << _PIN_INT_SHIFT)
#define _PIN_DMA               (1 << _PIN_INT_SHIFT)
#define _PIN_INTERRUPT         (2 << _PIN_INT_SHIFT)

#define PIN_DMA_RISING         (5  << _PIN_INT_SHIFT) /* 00101 DMA Request on rising edge */
#define PIN_DMA_FALLING        (9  << _PIN_INT_SHIFT) /* 01001 DMA Request on falling edge */
#define PIN_DMA_BOTH           (13 << _PIN_INT_SHIFT) /* 01101 DMA Request on either edge */
#define PIN_INT_ZERO           (2  << _PIN_INT_SHIFT) /* 00010 Interrupt when logic zero */
#define PIN_INT_RISING         (6  << _PIN_INT_SHIFT) /* 00110 Interrupt on rising edge */
#define PIN_INT_FALLING        (10 << _PIN_INT_SHIFT) /* 01010 Interrupt on falling edge */
#define PIN_INT_BOTH           (14 << _PIN_INT_SHIFT) /* 01110 Interrupt on either edge */
#define PIN_INT_ONE            (18 << _PIN_INT_SHIFT) /* 10010 Interrupt when logic one */

/* Two bits is used to enable the filter options:
 *
 * ---- ---- ---- -fd- ---- ---- ---- ----
 *
 * Passive Filter and digital filter enable are valid in all digital pin
 * muxing modes.
 */

#define PIN_PASV_FILTER        (1 << 18)  /* Bit 18: Enable passive filter */
#define PIN_DIG_FILTER         (1 << 17)  /* Bit 17: Enable digital filter */
 
/* Three bits are used to define the port number:
 *
 * ---- ---- ---- ---- ---- -ppp ---- ----
 */

#define _PIN_PORT_SHIFT        (8)  /* Bits 8-10: port number */
#define _PIN_PORT_MASK         (7 << _PIN_PORT_SHIFT)

#define PIN_PORTA              (KL_PORTA << _PIN_PORT_SHIFT)
#define PIN_PORTB              (KL_PORTB << _PIN_PORT_SHIFT)
#define PIN_PORTC              (KL_PORTC << _PIN_PORT_SHIFT)
#define PIN_PORTD              (KL_PORTD << _PIN_PORT_SHIFT)
#define PIN_PORTE              (KL_PORTE << _PIN_PORT_SHIFT)

/* Five bits are used to define the pin number:
 *
 * ---- ---- ---- ---- ---- ---- ---b bbbb
 */

#define _PIN_SHIFT             (0)  /* Bits 0-4: port number */
#define _PIN_MASK              (31 << _PIN_SHIFT)

#define PIN(n)                 ((n) << _PIN_SHIFT)
#define PIN0                   (0 << _PIN_SHIFT)
#define PIN1                   (1 << _PIN_SHIFT)
#define PIN2                   (2 << _PIN_SHIFT)
#define PIN3                   (3 << _PIN_SHIFT)
#define PIN4                   (4 << _PIN_SHIFT)
#define PIN5                   (5 << _PIN_SHIFT)
#define PIN6                   (6 << _PIN_SHIFT)
#define PIN7                   (7 << _PIN_SHIFT)
#define PIN8                   (8 << _PIN_SHIFT)
#define PIN9                   (9 << _PIN_SHIFT)
#define PIN10                  (10 << _PIN_SHIFT)
#define PIN11                  (11 << _PIN_SHIFT)
#define PIN12                  (12 << _PIN_SHIFT)
#define PIN13                  (13 << _PIN_SHIFT)
#define PIN14                  (14 << _PIN_SHIFT)
#define PIN15                  (15 << _PIN_SHIFT)
#define PIN16                  (16 << _PIN_SHIFT)
#define PIN17                  (17 << _PIN_SHIFT)
#define PIN18                  (18 << _PIN_SHIFT)
#define PIN19                  (19 << _PIN_SHIFT)
#define PIN20                  (20 << _PIN_SHIFT)
#define PIN21                  (21 << _PIN_SHIFT)
#define PIN22                  (22 << _PIN_SHIFT)
#define PIN23                  (23 << _PIN_SHIFT)
#define PIN24                  (24 << _PIN_SHIFT)
#define PIN25                  (25 << _PIN_SHIFT)
#define PIN26                  (26 << _PIN_SHIFT)
#define PIN27                  (27 << _PIN_SHIFT)
#define PIN28                  (28 << _PIN_SHIFT)
#define PIN29                  (29 << _PIN_SHIFT)
#define PIN30                  (30 << _PIN_SHIFT)
#define PIN31                  (31 << _PIN_SHIFT)

/************************************************************************************
 * Public Types
 ************************************************************************************/

typedef FAR void *DMA_HANDLE;
typedef void (*dma_callback_t)(DMA_HANDLE handle, void *arg, int result);

/* The following is used for sampling DMA registers when CONFIG DEBUG_DMA is selected */

#ifdef CONFIG_DEBUG_DMA
struct kl_dmaglobalregs_s
{
#warning "Missing logic"
  /* Global Registers */
};

struct kl_dmachanregs_s
{
#warning "Missing logic"
  /* Channel Registers */
};

struct kl_dmaregs_s
{
  /* Global Registers */

  struct kl_dmaglobalregs_s gbl;

  /* Channel Registers */

  struct kl_dmachanregs_s   ch;
};
#endif

/************************************************************************************
 * Inline Functions
 ************************************************************************************/

#ifndef __ASSEMBLY__

/************************************************************************************
 * Public Data
 ************************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/************************************************************************************
 * Public Function Prototypes
 ************************************************************************************/

/************************************************************************************
 * Name: kl_clockconfig
 *
 * Description:
 *   Called to initialize the Kinetis chip.  This does whatever setup is needed to
 *   put the  MCU in a usable state.  This includes the initialization of clocking
 *   using the settings in board.h.
 *
 ************************************************************************************/

void kl_clockconfig(void);

/************************************************************************************
 * Name: kl_lowsetup
 *
 * Description:
 *   Called at the very beginning of _start.  Performs low level initialization
 *   including setup of the console UART.  This UART done early so that the serial
 *   console is available for debugging very early in the boot sequence.
 *
 ************************************************************************************/

void kl_lowsetup(void);

/******************************************************************************
 * Name: kl_uartreset
 *
 * Description:
 *   Reset a UART.
 *
 ******************************************************************************/

#ifdef HAVE_UART_DEVICE
void kl_uartreset(uintptr_t uart_base);
#endif

/******************************************************************************
 * Name: kl_uartconfigure
 *
 * Description:
 *   Configure a UART as a RS-232 UART.
 *
 ******************************************************************************/

#ifdef HAVE_UART_DEVICE
void kl_uartconfigure(uintptr_t uart_base, uint32_t baud, uint32_t clock,
                      unsigned int parity, unsigned int nbits);
#endif

/************************************************************************************
 * Name: kl_wddisable
 *
 * Description:
 *   Disable the watchdog timer
 *
 ************************************************************************************/

void kl_wddisable(void);

/************************************************************************************
 * Name: kl_pinconfig
 *
 * Description:
 *   Configure a pin based on bit-encoded description of the pin.
 *
 ************************************************************************************/

int kl_pinconfig(uint32_t cfgset);

/************************************************************************************
 * Name: kl_pinfilter
 *
 * Description:
 *   Configure the digital filter associated with a port. The digital filter
 *   capabilities of the PORT module are available in all digital pin muxing modes.
 *
 * Input parmeters:
 *   port  - See KL_PORTn definitions in kl_port.h
 *   lpo   - true: Digital Filters are clocked by the bus clock
 *           false: Digital Filters are clocked by the 1 kHz LPO clock
 *   width - Filter Length
 *
 ************************************************************************************/

int kl_pinfilter(unsigned int port, bool lpo, unsigned int width);

/************************************************************************************
 * Name: kl_gpiowrite
 *
 * Description:
 *   Write one or zero to the selected GPIO pin
 *
 ************************************************************************************/

void kl_gpiowrite(uint32_t pinset, bool value);

/************************************************************************************
 * Name: kl_gpioread
 *
 * Description:
 *   Read one or zero from the selected GPIO pin
 *
 ************************************************************************************/

bool kl_gpioread(uint32_t pinset);

/************************************************************************************
 * Name: kl_pinirqinitialize
 *
 * Description:
 *   Initialize logic to support a second level of interrupt decoding for GPIO pins.
 *
 ************************************************************************************/

#ifdef CONFIG_GPIO_IRQ
void kl_pinirqinitialize(void);
#else
#  define kl_pinirqinitialize()
#endif

/************************************************************************************
 * Name: kl_pinirqattach
 *
 * Description:
 *   Attach a pin interrupt handler.  The normal initalization sequence is:
 *
 *   1. Call kl_pinconfig() to configure the interrupting pin (pin interrupts
 *      will be disabled.
 *   2. Call kl_pinirqattach() to attach the pin interrupt handling function.
 *   3. Call kl_pinirqenable() to enable interrupts on the pin.
 *
 * Parameters:
 *  - pinset:  Pin configuration
 *  - pinisr:  Pin interrupt service routine
 *
 * Returns:
 *   The previous value of the interrupt handler function pointer.  This value may,
 *   for example, be used to restore the previous handler when multiple handlers are
 *   used.
 *
 ************************************************************************************/

xcpt_t kl_pinirqattach(uint32_t pinset, xcpt_t pinisr);

/************************************************************************************
 * Name: kl_pinirqenable
 *
 * Description:
 *   Enable the interrupt for specified pin IRQ
 *
 ************************************************************************************/

#ifdef CONFIG_GPIO_IRQ
void kl_pinirqenable(uint32_t pinset);
#else
#  define kl_pinirqenable(pinset)
#endif

/************************************************************************************
 * Name: kl_pinirqdisable
 *
 * Description:
 *   Disable the interrupt for specified pin
 *
 ************************************************************************************/

#ifdef CONFIG_GPIO_IRQ
void kl_pinirqdisable(uint32_t pinset);
#else
#  define kl_pinirqdisable(pinset)
#endif

/************************************************************************************
 * Name: kl_pindmaenable
 *
 * Description:
 *   Enable DMA for specified pin
 *
 ************************************************************************************/

#ifdef CONFIG_KL_DMA
void kl_pindmaenable(uint32_t pinset);
#endif

/************************************************************************************
 * Name: kl_pindmadisable
 *
 * Description:
 *   Disable DMA for specified pin
 *
 ************************************************************************************/

#ifdef CONFIG_KL_DMA
void kl_pindmadisable(uint32_t pinset);
#endif

/************************************************************************************
 * Function:  kl_pindump
 *
 * Description:
 *   Dump all GPIO registers associated with the base address of the provided pinset.
 *
 ************************************************************************************/

#ifdef CONFIG_DEBUG_GPIO
int kl_pindump(uint32_t pinset, const char *msg);
#else
#  define kl_pindump(p,m)
#endif

/************************************************************************************
 * Name: kl_clrpend
 *
 * Description:
 *   Clear a pending interrupt at the NVIC.  This does not seem to be required
 *   for most interrupts.
 *
 ************************************************************************************/

void kl_clrpend(int irq);

/************************************************************************************
 * Name:  kl_spi[n]select, kl_spi[n]status, and kl_spi[n]cmddata
 *
 * Description:
 *   These external functions must be provided by board-specific logic.  They are
 *   implementations of the select, status, and cmddata methods of the SPI interface
 *   defined by struct spi_ops_s (see include/nuttx/spi.h). All other methods 
 *   including up_spiinitialize()) are provided by common Kinetis logic.  To use
 *   this common SPI logic on your board:
 *
 *   1. Provide logic in kl_boardinitialize() to configure SPI chip select
 *      pins.
 *   2. Provide kl_spi[n]select() and kl_spi[n]status() functions
 *      in your board-specific logic.  These functions will perform chip selection
 *      and status operations using GPIOs in the way your board is configured.
 *   2. If CONFIG_SPI_CMDDATA is defined in the NuttX configuration, provide
 *      kl_spi[n]cmddata() functions in your board-specific logic.  These
 *      functions will perform cmd/data selection operations using GPIOs in the way
 *      your board is configured.
 *   3. Add a call to up_spiinitialize() in your low level application
 *      initialization logic
 *   4. The handle returned by up_spiinitialize() may then be used to bind the
 *      SPI driver to higher level logic (e.g., calling 
 *      mmcsd_spislotinitialize(), for example, will bind the SPI driver to
 *      the SPI MMC/SD driver).
 *
 ************************************************************************************/

struct spi_dev_s;
enum spi_dev_e;

#ifdef CONFIG_KL_SPI0
void  kl_spi0select(FAR struct spi_dev_s *dev, enum spi_dev_e devid, bool selected);
uint8_t kl_spi0status(FAR struct spi_dev_s *dev, enum spi_dev_e devid);
#ifdef CONFIG_SPI_CMDDATA
int kl_spi0cmddata(FAR struct spi_dev_s *dev, enum spi_dev_e devid, bool cmd);
#endif
#endif
#ifdef CONFIG_KL_SPI1
void  kl_spi1select(FAR struct spi_dev_s *dev, enum spi_dev_e devid, bool selected);
uint8_t kl_spi1status(FAR struct spi_dev_s *dev, enum spi_dev_e devid);
#ifdef CONFIG_SPI_CMDDATA
int kl_spi1cmddata(FAR struct spi_dev_s *dev, enum spi_dev_e devid, bool cmd);
#endif
#endif
#ifdef CONFIG_KL_SPI2
void  kl_spi2select(FAR struct spi_dev_s *dev, enum spi_dev_e devid, bool selected);
uint8_t kl_spi2status(FAR struct spi_dev_s *dev, enum spi_dev_e devid);
#ifdef CONFIG_SPI_CMDDATA
int kl_spi2cmddata(FAR struct spi_dev_s *dev, enum spi_dev_e devid, bool cmd);
#endif
#endif

/****************************************************************************
 * Name: ssp_flush
 *
 * Description:
 *   Flush and discard any words left in the RX fifo.  This can be called
 *   from spi[n]select after a device is deselected (if you worry about such
 *   things).
 *
 * Input Parameters:
 *   dev - Device-specific state data
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#if defined(CONFIG_KL_SPI0) || defined(CONFIG_KL_SPI0) || defined(CONFIG_KL_SPI2)
struct spi_dev_s;
void spi_flush(FAR struct spi_dev_s *dev);
#endif

/****************************************************************************
 * Name: kl_dmainitialize
 *
 * Description:
 *   Initialize the GPDMA subsystem.
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#ifdef CONFIG_KL_DMA
void kl_dmainitilaize(void);
#endif

/****************************************************************************
 * Name: kl_dmachannel
 *
 * Description:
 *   Allocate a DMA channel.  This function sets aside a DMA channel and
 *   gives the caller exclusive access to the DMA channel.
 *
 * Returned Value:
 *   One success, this function returns a non-NULL, void* DMA channel
 *   handle.  NULL is returned on any failure.  This function can fail only
 *   if no DMA channel is available.
 *
 ****************************************************************************/

#ifdef CONFIG_KL_DMA
DMA_HANDLE kl_dmachannel(void);
#endif

/****************************************************************************
 * Name: kl_dmafree
 *
 * Description:
 *   Release a DMA channel.  NOTE:  The 'handle' used in this argument must
 *   NEVER be used again until kl_dmachannel() is called again to re-gain
 *   a valid handle.
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#ifdef CONFIG_KL_DMA
void kl_dmafree(DMA_HANDLE handle);
#endif

/****************************************************************************
 * Name: kl_dmasetup
 *
 * Description:
 *   Configure DMA for one transfer.
 *
 ****************************************************************************/

#ifdef CONFIG_KL_DMA
int kl_dmarxsetup(DMA_HANDLE handle, uint32_t control, uint32_t config,
                  uint32_t srcaddr, uint32_t destaddr, size_t nbytes);
#endif

/****************************************************************************
 * Name: kl_dmastart
 *
 * Description:
 *   Start the DMA transfer
 *
 ****************************************************************************/

#ifdef CONFIG_KL_DMA
int kl_dmastart(DMA_HANDLE handle, dma_callback_t callback, void *arg);
#endif

/****************************************************************************
 * Name: kl_dmastop
 *
 * Description:
 *   Cancel the DMA.  After kl_dmastop() is called, the DMA channel is
 *   reset and kl_dmasetup() must be called before kl_dmastart() can be
 *   called again
 *
 ****************************************************************************/

#ifdef CONFIG_KL_DMA
void kl_dmastop(DMA_HANDLE handle);
#endif

/****************************************************************************
 * Name: kl_dmasample
 *
 * Description:
 *   Sample DMA register contents
 *
 ****************************************************************************/

#ifdef CONFIG_KL_DMA
#ifdef CONFIG_DEBUG_DMA
void kl_dmasample(DMA_HANDLE handle, struct kl_dmaregs_s *regs);
#else
#  define kl_dmasample(handle,regs)
#endif
#endif

/****************************************************************************
 * Name: kl_dmadump
 *
 * Description:
 *   Dump previously sampled DMA register contents
 *
 ****************************************************************************/

#ifdef CONFIG_KL_DMA
#ifdef CONFIG_DEBUG_DMA
void kl_dmadump(DMA_HANDLE handle, const struct kl_dmaregs_s *regs,
                const char *msg);
#else
#  define kl_dmadump(handle,regs,msg)
#endif
#endif

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_KL_KL_INTERNAL_H */
