/****************************************************************************
 * arch/arm/src/samv7/sam_port.c
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
#include <time.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/spinlock.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "sam_gpio.h"
#include "hardware/sam_port.h"

#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static spinlock_t g_port_lock = SP_UNLOCKED;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpioinit
 *
 * Description:
 *   Based on configuration within the .config file, it does:
 *    - Remaps positions of alternative functions for GPIO.
 *
 *   Typically called from sam_start().
 *
 ****************************************************************************/

void sam_gpioinit(void)
{
  /* Nothing to do for CA90 PORT */
}

/****************************************************************************
 * Name: sam_configgpio
 *
 * Description:
 *   Configure a GPIO pin based on bit-encoded description of the pin.
 *
 ****************************************************************************/

int sam_configgpio(gpio_pinset_t cfgset)
{
  int port = sam_gpio_port(cfgset);
  int pin  = sam_gpio_pin(cfgset);
  uintptr_t base = SAM_PORTN_BASE(port);
  irqstate_t flags;
  uint8_t pincfg = 0;
  uint32_t mode = cfgset & GPIO_MODE_MASK;

  /* Disable interrupts to prohibit re-entrance. */

  flags = spin_lock_irqsave(&g_port_lock);

  /* Decode PINCFG */

  if ((cfgset & GPIO_CFG_PULLUP) != 0 || (cfgset & GPIO_CFG_PULLDOWN) != 0)
    {
      pincfg |= PORT_PINCFG_PULLEN;
    }

  if (mode == GPIO_INPUT || mode == GPIO_ALTERNATE)
    {
      pincfg |= PORT_PINCFG_INEN;
    }

  if (mode >= GPIO_PERIPHA && mode <= GPIO_PERIPHD)
    {
      pincfg |= PORT_PINCFG_PMUXEN;

      /* Set PMUX */

      uintptr_t pmux_addr = base + SAM_PORT_PMUX_OFFSET(pin >> 1);
      uint8_t pmux = getreg8(pmux_addr);
      uint8_t mux_val = (mode - GPIO_PERIPHA); /* Map PERIPHA..D to 0..3 (Mux A..D) */

      if ((pin & 1) != 0)
        {
          pmux &= ~PORT_PMUX_ODD_MASK;
          pmux |= PORT_PMUX_ODD(mux_val);
        }
      else
        {
          pmux &= ~PORT_PMUX_EVEN_MASK;
          pmux |= PORT_PMUX_EVEN(mux_val);
        }
      putreg8(pmux, pmux_addr);
    }

  /* Set direction and initial output value if it's an output */

  if (mode == GPIO_OUTPUT)
    {
      if ((cfgset & GPIO_OUTPUT_SET) != 0)
        {
          putreg32(1 << pin, base + SAM_PORT_OUTSET_OFFSET);
        }
      else
        {
          putreg32(1 << pin, base + SAM_PORT_OUTCLR_OFFSET);
        }
      putreg32(1 << pin, base + SAM_PORT_DIRSET_OFFSET);
    }
  else
    {
      putreg32(1 << pin, base + SAM_PORT_DIRCLR_OFFSET);
    }

  /* Write PINCFG */

  putreg8(pincfg, base + SAM_PORT_PINCFG_OFFSET(pin));

  spin_unlock_irqrestore(&g_port_lock, flags);
  return OK;
}

/****************************************************************************
 * Name: sam_gpiowrite
 *
 * Description:
 *   Write one or zero to the selected GPIO pin
 *
 ****************************************************************************/

void sam_gpiowrite(gpio_pinset_t pinset, bool value)
{
  int port = sam_gpio_port(pinset);
  int pin  = sam_gpio_pin(pinset);
  uintptr_t base = SAM_PORTN_BASE(port);

  if (value)
    {
      putreg32(1 << pin, base + SAM_PORT_OUTSET_OFFSET);
    }
  else
    {
      putreg32(1 << pin, base + SAM_PORT_OUTCLR_OFFSET);
    }
}

/****************************************************************************
 * Name: sam_gpioread
 *
 * Description:
 *   Read one or zero from the selected GPIO pin
 *
 ****************************************************************************/

bool sam_gpioread(gpio_pinset_t pinset)
{
  int port = sam_gpio_port(pinset);
  int pin  = sam_gpio_pin(pinset);
  uintptr_t base = SAM_PORTN_BASE(port);

  return (getreg32(base + SAM_PORT_IN_OFFSET) & (1 << pin)) != 0;
}

#endif /* CONFIG_ARCH_CHIP_PIC32CZCA90 */
