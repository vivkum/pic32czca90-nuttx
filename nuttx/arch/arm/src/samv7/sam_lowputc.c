/****************************************************************************
 * arch/arm/src/samv7/sam_lowputc.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/spinlock.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "sam_config.h"
#include "sam_gpio.h"
#include "sam_periphclks.h"
#include "sam_start.h"

#include "hardware/sam_uart.h"
#include "hardware/sam_pinmap.h"
#include "hardware/sam_matrix.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#ifdef HAVE_SERIAL_CONSOLE

/* BAUD definitions
 *
 * The source clock is selectable and could be one of:
 *
 *   - The peripheral clock
 *   - A division of the peripheral clock, where the divider is product-
 *     dependent, but generally set to 8
 *   - A processor/peripheral independent clock source fully programmable
 *      provided by PMC (PCK)
 *   - The external clock, available on the SCK pin
 *
 * Only the first two options are supported by this driver.  The divided
 * peripheral clock is only used for very low BAUD selections.
 */

#define FAST_USART_CLOCK   BOARD_MCK_FREQUENCY
#define SLOW_USART_CLOCK   (BOARD_MCK_FREQUENCY >> 3)

/* Select USART parameters for the selected console */

#  if defined(CONFIG_UART0_SERIAL_CONSOLE)
#    define SAM_CONSOLE_BASE     SAM_UART0_BASE
#    define SAM_CONSOLE_BAUD     CONFIG_UART0_BAUD
#    define SAM_CONSOLE_BITS     CONFIG_UART0_BITS
#    define SAM_CONSOLE_PARITY   CONFIG_UART0_PARITY
#    define SAM_CONSOLE_2STOP    CONFIG_UART0_2STOP
#  elif defined(CONFIG_UART1_SERIAL_CONSOLE)
#    define SAM_CONSOLE_BASE     SAM_UART1_BASE
#    define SAM_CONSOLE_BAUD     CONFIG_UART1_BAUD
#    define SAM_CONSOLE_BITS     CONFIG_UART1_BITS
#    define SAM_CONSOLE_PARITY   CONFIG_UART1_PARITY
#    define SAM_CONSOLE_2STOP    CONFIG_UART1_2STOP
#  elif defined(CONFIG_UART2_SERIAL_CONSOLE)
#    define SAM_CONSOLE_BASE     SAM_UART2_BASE
#    define SAM_CONSOLE_BAUD     CONFIG_UART2_BAUD
#    define SAM_CONSOLE_BITS     CONFIG_UART2_BITS
#    define SAM_CONSOLE_PARITY   CONFIG_UART2_PARITY
#    define SAM_CONSOLE_2STOP    CONFIG_UART2_2STOP
#  elif defined(CONFIG_UART3_SERIAL_CONSOLE)
#    define SAM_CONSOLE_BASE     SAM_UART3_BASE
#    define SAM_CONSOLE_BAUD     CONFIG_UART3_BAUD
#    define SAM_CONSOLE_BITS     CONFIG_UART3_BITS
#    define SAM_CONSOLE_PARITY   CONFIG_UART3_PARITY
#    define SAM_CONSOLE_2STOP    CONFIG_UART3_2STOP
#  elif defined(CONFIG_UART4_SERIAL_CONSOLE)
#    define SAM_CONSOLE_BASE     SAM_UART4_BASE
#    define SAM_CONSOLE_BAUD     CONFIG_UART4_BAUD
#    define SAM_CONSOLE_BITS     CONFIG_UART4_BITS
#    define SAM_CONSOLE_PARITY   CONFIG_UART4_PARITY
#    define SAM_CONSOLE_2STOP    CONFIG_UART4_2STOP
#  elif defined(CONFIG_USART0_SERIAL_CONSOLE)
#    define SAM_CONSOLE_BASE     SAM_USART0_BASE
#    define SAM_CONSOLE_BAUD     CONFIG_USART0_BAUD
#    define SAM_CONSOLE_BITS     CONFIG_USART0_BITS
#    define SAM_CONSOLE_PARITY   CONFIG_USART0_PARITY
#    define SAM_CONSOLE_2STOP    CONFIG_USART0_2STOP
#  elif defined(CONFIG_USART1_SERIAL_CONSOLE)
#    define SAM_CONSOLE_BASE     SAM_USART1_BASE
#    define SAM_CONSOLE_BAUD     CONFIG_USART1_BAUD
#    define SAM_CONSOLE_BITS     CONFIG_USART1_BITS
#    define SAM_CONSOLE_PARITY   CONFIG_USART1_PARITY
#    define SAM_CONSOLE_2STOP    CONFIG_USART1_2STOP
#  elif defined(CONFIG_USART2_SERIAL_CONSOLE)
#    define SAM_CONSOLE_BASE     SAM_USART2_BASE
#    define SAM_CONSOLE_BAUD     CONFIG_USART2_BAUD
#    define SAM_CONSOLE_BITS     CONFIG_USART2_BITS
#    define SAM_CONSOLE_PARITY   CONFIG_USART2_PARITY
#    define SAM_CONSOLE_2STOP    CONFIG_USART2_2STOP
#  else
#    error "No CONFIG_U[S]ARTn_SERIAL_CONSOLE Setting"
#  endif

/* Select the settings for the mode register */

#  if SAM_CONSOLE_BITS == 5
#    define MR_CHRL_VALUE UART_MR_CHRL_5BITS /* 5 bits */
#  elif SAM_CONSOLE_BITS == 6
#    define MR_CHRL_VALUE UART_MR_CHRL_6BITS  /* 6 bits */
#  elif SAM_CONSOLE_BITS == 7
#    define MR_CHRL_VALUE UART_MR_CHRL_7BITS /* 7 bits */
#  elif SAM_CONSOLE_BITS == 8
#    define MR_CHRL_VALUE UART_MR_CHRL_8BITS /* 8 bits */
#  elif SAM_CONSOLE_BITS == 9 && !defined(CONFIG_UART0_SERIAL_CONSOLE) && \
       !defined(CONFIG_UART1_SERIAL_CONSOLE)
#    define MR_CHRL_VALUE UART_MR_MODE9
#  else
#    error "Invalid number of bits"
#  endif

#  if SAM_CONSOLE_PARITY == 1
#    define MR_PAR_VALUE UART_MR_PAR_ODD
#  elif SAM_CONSOLE_PARITY == 2
#    define MR_PAR_VALUE UART_MR_PAR_EVEN
#  else
#    define MR_PAR_VALUE UART_MR_PAR_NONE
#  endif

#  if SAM_CONSOLE_2STOP != 0
#    define MR_NBSTOP_VALUE UART_MR_NBSTOP_2
#  else
#    define MR_NBSTOP_VALUE UART_MR_NBSTOP_1
#  endif

#  define MR_VALUE (UART_MR_MODE_NORMAL | UART_MR_USCLKS_MCK | \
                    MR_CHRL_VALUE | MR_PAR_VALUE | MR_NBSTOP_VALUE)

#endif /* HAVE_SERIAL_CONSOLE */

/****************************************************************************
 * Private Data
 ****************************************************************************/

#ifdef HAVE_SERIAL_CONSOLE
static spinlock_t g_sam_lowputc_lock = SP_UNLOCKED;
#endif

/****************************************************************************
 * CA90-specific early SERCOM1 USART console
 *
 * Virtual COM port: PC4 = SERCOM1_PAD0 (TX, mux D)
 *                  PC7 = SERCOM1_PAD3 (RX, mux D)
 * Clock: GCLK0 (DFLL 48 MHz after boot ROM)
 * Baud:  115200  →  BAUD register = 63019
 *
 * Register map (all offsets from SERCOM1_BASE = 0x46002000):
 *   CTRLA   +0x00  (32-bit)
 *   CTRLB   +0x04  (32-bit)
 *   BAUD    +0x0C  (16-bit)
 *   INTFLAG +0x18  (8-bit)  bit0=DRE, bit1=TXC, bit2=RXC
 *   DATA    +0x28  (16-bit)
 *
 * MCLK: base 0x44052000, CLKMSK0 at +0x3C  → bit 24 = SERCOM1
 * GCLK: base 0x44050000, PCHCTRL[24] at +0xE0 → GEN=0, CHEN=1
 * PORT: base 0x44840000, group C at +0x100
 *   PMUX[2]   +0x132  PC4 even nibble = D(3)
 *   PMUX[3]   +0x133  PC7 odd  nibble = D(3)<<4
 *   PINCFG[4] +0x144  PMUXEN(bit0)=1
 *   PINCFG[7] +0x147  PMUXEN(bit0)=1
 ****************************************************************************/

#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90

#define CA90_MCLK_BASE          0x44052000u
#define CA90_MCLK_CLKMSK0       (CA90_MCLK_BASE + 0x3Cu)
#define CA90_MCLK_SERCOM1_BIT   (1u << 24)

#define CA90_GCLK_BASE          0x44050000u
#define CA90_GCLK_SYNCBUSY      (CA90_GCLK_BASE + 0x04u)
#define CA90_GCLK_PCHCTRL_SERCOM1 (CA90_GCLK_BASE + 0x80u + (24u * 4u))
#define CA90_GCLK_PCHCTRL_CHEN  (1u << 6)

#define CA90_PORTC_BASE         (0x44840000u + 0x100u)
#define CA90_PORTC_PMUX2        (CA90_PORTC_BASE + 0x32u)  /* PC4/PC5 */
#define CA90_PORTC_PMUX3        (CA90_PORTC_BASE + 0x33u)  /* PC6/PC7 */
#define CA90_PORTC_PINCFG4      (CA90_PORTC_BASE + 0x44u)  /* PC4 */
#define CA90_PORTC_PINCFG7      (CA90_PORTC_BASE + 0x47u)  /* PC7 */
#define CA90_PORT_PMUX_D        3u   /* peripheral function D */
#define CA90_PORT_PMUXEN        (1u << 0)

#define CA90_SERCOM1_BASE       0x46002000u
#define CA90_SERCOM1_CTRLA      (CA90_SERCOM1_BASE + 0x00u)
#define CA90_SERCOM1_CTRLB      (CA90_SERCOM1_BASE + 0x04u)
#define CA90_SERCOM1_BAUD       (CA90_SERCOM1_BASE + 0x0Cu)
#define CA90_SERCOM1_INTFLAG    (CA90_SERCOM1_BASE + 0x18u)
#define CA90_SERCOM1_DATA       (CA90_SERCOM1_BASE + 0x28u)
#define CA90_SERCOM1_SYNCBUSY   (CA90_SERCOM1_BASE + 0x1Cu)

/* CTRLA bits */
#define SERCOM_USART_CTRLA_SWRST   (1u << 0)
#define SERCOM_USART_CTRLA_ENABLE  (1u << 1)
#define SERCOM_USART_CTRLA_MODE(x) ((x) << 2)   /* 1 = USART internal clk */
#define SERCOM_USART_CTRLA_RXPO(x) ((x) << 20)  /* 3 = PAD3 */
#define SERCOM_USART_CTRLA_TXPO(x) ((x) << 16)  /* 0 = PAD0 TX */
#define SERCOM_USART_CTRLA_DORD    (1u << 30)    /* LSB first */

/* CTRLB bits */
#define SERCOM_USART_CTRLB_CHSIZE(x) ((x) << 0) /* 0=8-bit */
#define SERCOM_USART_CTRLB_TXEN      (1u << 16)
#define SERCOM_USART_CTRLB_RXEN      (1u << 17)

/* INTFLAG bits */
#define SERCOM_USART_INTFLAG_DRE  (1u << 0)

/* SYNCBUSY bits */
#define SERCOM_USART_SYNCBUSY_SWRST  (1u << 0)
#define SERCOM_USART_SYNCBUSY_ENABLE (1u << 1)
#define SERCOM_USART_SYNCBUSY_CTRLB  (1u << 2)

/* BAUD value: 65536 * (1 - 16 * 115200 / 48000000) = 63019
 * Assumes GCLK0 = DFLL = 48 MHz after boot ROM.
 */
#define CA90_SERCOM1_BAUD_115200  63019u

static void ca90_sercom1_init(void)
{
  /* 1. Enable MCLK APB clock for SERCOM1 (bit 24 of CLKMSK0) */

  putreg32(getreg32(CA90_MCLK_CLKMSK0) | CA90_MCLK_SERCOM1_BIT,
           CA90_MCLK_CLKMSK0);

  /* 2. Route GCLK0 (48 MHz DFLL) to SERCOM1 core clock */

  putreg32(CA90_GCLK_PCHCTRL_CHEN,   /* GEN=0 (GCLK0), CHEN=1 */
           CA90_GCLK_PCHCTRL_SERCOM1);
  while (getreg32(CA90_GCLK_SYNCBUSY) & (1u << 2));  /* wait SYNCBUSY */

  /* 3. Pin mux: PC4 → SERCOM1_PAD0 (TX), PC7 → SERCOM1_PAD3 (RX)
   *    PMUX[2] lower nibble = PC4, PMUX[3] upper nibble = PC7 (both mux D)
   */

  putreg8((uint8_t)(CA90_PORT_PMUX_D | (CA90_PORT_PMUX_D << 4)),
          CA90_PORTC_PMUX2);  /* PC4=D, PC5=D (PC5 unused, safe) */
  putreg8((uint8_t)(CA90_PORT_PMUX_D | (CA90_PORT_PMUX_D << 4)),
          CA90_PORTC_PMUX3);  /* PC6=D (unused), PC7=D */
  putreg8((uint8_t)CA90_PORT_PMUXEN, CA90_PORTC_PINCFG4);
  putreg8((uint8_t)CA90_PORT_PMUXEN, CA90_PORTC_PINCFG7);

  /* 4. Software-reset SERCOM1 */

  putreg32(SERCOM_USART_CTRLA_SWRST, CA90_SERCOM1_CTRLA);
  while (getreg32(CA90_SERCOM1_SYNCBUSY) & SERCOM_USART_SYNCBUSY_SWRST);

  /* 5. Configure USART: internal clock, TX=PAD0, RX=PAD3, LSB first */

  putreg32(SERCOM_USART_CTRLA_MODE(1) |
           SERCOM_USART_CTRLA_RXPO(3) |
           SERCOM_USART_CTRLA_TXPO(0) |
           SERCOM_USART_CTRLA_DORD,
           CA90_SERCOM1_CTRLA);

  /* 6. BAUD = 63019 for 115200 @ 48 MHz */

  putreg16((uint16_t)CA90_SERCOM1_BAUD_115200, CA90_SERCOM1_BAUD);

  /* 7. Enable TX and RX */

  putreg32(SERCOM_USART_CTRLB_CHSIZE(0) |
           SERCOM_USART_CTRLB_TXEN |
           SERCOM_USART_CTRLB_RXEN,
           CA90_SERCOM1_CTRLB);
  while (getreg32(CA90_SERCOM1_SYNCBUSY) & SERCOM_USART_SYNCBUSY_CTRLB);

  /* 8. Enable SERCOM1 */

  putreg32(getreg32(CA90_SERCOM1_CTRLA) | SERCOM_USART_CTRLA_ENABLE,
           CA90_SERCOM1_CTRLA);
  while (getreg32(CA90_SERCOM1_SYNCBUSY) & SERCOM_USART_SYNCBUSY_ENABLE);
}

static void ca90_sercom1_putc(char ch)
{
  while ((getreg8(CA90_SERCOM1_INTFLAG) & SERCOM_USART_INTFLAG_DRE) == 0);
  putreg16((uint16_t)(unsigned char)ch, CA90_SERCOM1_DATA);
}

#endif /* CONFIG_ARCH_CHIP_PIC32CZCA90 */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_lowputc
 *
 * Description:
 *   Output one byte on the serial console
 *
 ****************************************************************************/

void arm_lowputc(char ch)
{
#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90
  ca90_sercom1_putc(ch);
#elif defined(HAVE_SERIAL_CONSOLE)
  irqstate_t flags;

  /* Wait for the transmitter to be available */

  flags = spin_lock_irqsave(&g_sam_lowputc_lock);
  while ((getreg32(SAM_CONSOLE_BASE + SAM_UART_SR_OFFSET) &
    UART_INT_TXEMPTY) == 0);

  /* Send the character */

  putreg32((uint32_t)ch, SAM_CONSOLE_BASE + SAM_UART_THR_OFFSET);

  spin_unlock_irqrestore(&g_sam_lowputc_lock, flags);
#endif
}

/****************************************************************************
 * Name: up_putc
 *
 * Description:
 *   Provide priority, low-level access to support OS debug writes
 *
 ****************************************************************************/

void up_putc(int ch)
{
#ifdef HAVE_SERIAL_CONSOLE
  arm_lowputc(ch);
#endif
}

/****************************************************************************
 * Name: sam_lowsetup
 *
 * Description:
 *   This performs basic initialization of the USART used for the serial
 *   console.  Its purpose is to get the console output available as soon
 *   as possible.
 *
 ****************************************************************************/

void sam_lowsetup(void)
{
#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90
  /* CA90: initialize SERCOM1 USART on PC4 (TX) / PC7 (RX) */

  ca90_sercom1_init();
  return;
#endif

#if defined(HAVE_SERIAL_CONSOLE) && !defined(CONFIG_SUPPRESS_UART_CONFIG)
  uint64_t divb3;
  uint32_t intpart;
  uint32_t fracpart;
  uint32_t regval;
#endif

  /* Enable clocking for all selected UART/USARTs */

#ifdef CONFIG_UART0_SERIALDRIVER
  sam_uart0_enableclk();
#endif
#ifdef CONFIG_UART1_SERIALDRIVER
  sam_uart1_enableclk();
#endif
#ifdef CONFIG_UART2_SERIALDRIVER
  sam_uart2_enableclk();
#endif
#ifdef CONFIG_UART3_SERIALDRIVER
  sam_uart3_enableclk();
#endif
#ifdef CONFIG_UART4_SERIALDRIVER
  sam_uart4_enableclk();
#endif
#ifdef CONFIG_USART0_SERIALDRIVER
  sam_usart0_enableclk();
#endif
#ifdef CONFIG_USART1_SERIALDRIVER
  sam_usart1_enableclk();
#endif
#ifdef CONFIG_USART2_SERIALDRIVER
  sam_usart2_enableclk();
#endif

  /* Configure UART pins for all selected UART/USARTs */

#ifdef CONFIG_UART0_SERIALDRIVER
  sam_configgpio(GPIO_UART0_RXD);
  sam_configgpio(GPIO_UART0_TXD);
#endif

#ifdef CONFIG_UART1_SERIALDRIVER
  sam_configgpio(GPIO_UART1_RXD);
  sam_configgpio(GPIO_UART1_TXD);
#endif

#ifdef CONFIG_UART2_SERIALDRIVER
  sam_configgpio(GPIO_UART2_RXD);
  sam_configgpio(GPIO_UART2_TXD);
#endif

#ifdef CONFIG_UART3_SERIALDRIVER
  sam_configgpio(GPIO_UART3_RXD);
  sam_configgpio(GPIO_UART3_TXD);
#endif

#ifdef CONFIG_UART4_SERIALDRIVER
  sam_configgpio(GPIO_UART4_RXD);
  sam_configgpio(GPIO_UART4_TXD);
#endif

#ifdef CONFIG_USART0_SERIALDRIVER
  sam_configgpio(GPIO_USART0_RXD);
  sam_configgpio(GPIO_USART0_TXD);
#ifdef CONFIG_USART0_OFLOWCONTROL
  sam_configgpio(GPIO_USART0_CTS);
#endif
#ifdef CONFIG_USART0_IFLOWCONTROL
  sam_configgpio(GPIO_USART0_RTS);
#endif
#endif

#ifdef CONFIG_USART1_SERIALDRIVER
  sam_configgpio(GPIO_USART1_RXD);
  sam_configgpio(GPIO_USART1_TXD);
#  ifdef CONFIG_USART1_OFLOWCONTROL
  sam_configgpio(GPIO_USART1_CTS);
#  endif
#  ifdef CONFIG_USART1_IFLOWCONTROL
  sam_configgpio(GPIO_USART1_RTS);
#  endif

#endif

#ifdef CONFIG_USART2_SERIALDRIVER
  sam_configgpio(GPIO_USART2_RXD);
  sam_configgpio(GPIO_USART2_TXD);
#ifdef CONFIG_USART2_OFLOWCONTROL
  sam_configgpio(GPIO_USART2_CTS);
#endif
#ifdef CONFIG_USART2_IFLOWCONTROL
  sam_configgpio(GPIO_USART2_RTS);
#endif
#endif

  /* Configure the console (only) */
#if defined(HAVE_SERIAL_CONSOLE) && !defined(CONFIG_SUPPRESS_UART_CONFIG)
  /* Reset and disable receiver and transmitter */

  putreg32((UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS),
           SAM_CONSOLE_BASE + SAM_UART_CR_OFFSET);

  /* Disable all interrupts */

  putreg32(0xffffffff, SAM_CONSOLE_BASE + SAM_UART_IDR_OFFSET);

  /* Set up the mode register */

  putreg32(MR_VALUE, SAM_CONSOLE_BASE + SAM_UART_MR_OFFSET);

  /* Configure the console baud:
   *
   *   Fbaud   = USART_CLOCK / (16 * divisor)
   *   divisor = USART_CLOCK / (16 * Fbaud)
   *
   * NOTE: Oversampling by 8 is not supported. This may limit BAUD rates
   * for lower USART clocks.
   */

  divb3    = ((FAST_USART_CLOCK + (SAM_CONSOLE_BAUD << 3)) << 3) /
             (SAM_CONSOLE_BAUD << 4);
  intpart  = (divb3 >> 3);
  fracpart = (divb3 & 7);

  /* Retain the fast MR peripheral clock UNLESS unless using that clock
   * would result in an excessively large divider.
   *
   * REVISIT: The fractional divider is not used.
   */

  if ((intpart & ~UART_BRGR_CD_MASK) != 0)
    {
      /* Use the divided USART clock */

      divb3    = ((SLOW_USART_CLOCK + (SAM_CONSOLE_BAUD << 3)) << 3) /
                 (SAM_CONSOLE_BAUD << 4);
      intpart  = (divb3 >> 3);
      fracpart = (divb3 & 7);

      /* Re-select the clock source */

      regval  = getreg32(SAM_CONSOLE_BASE + SAM_UART_MR_OFFSET);
      regval &= ~UART_MR_USCLKS_MASK;
      regval |= UART_MR_USCLKS_MCKDIV;
      putreg32(regval, SAM_CONSOLE_BASE + SAM_UART_MR_OFFSET);
    }

  /* Save the BAUD divider (the fractional part is not used for UARTs) */

  regval = UART_BRGR_CD(intpart) | UART_BRGR_FP(fracpart);
  putreg32(regval, SAM_CONSOLE_BASE + SAM_UART_BRGR_OFFSET);

  /* Enable receiver & transmitter */

  putreg32((UART_CR_RXEN | UART_CR_TXEN),
           SAM_CONSOLE_BASE + SAM_UART_CR_OFFSET);
#endif
}
