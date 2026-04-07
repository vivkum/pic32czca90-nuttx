/****************************************************************************
 * arch/arm/src/samv7/hardware/sam_port.h
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

#ifndef __ARCH_ARM_SRC_SAMV7_HARDWARE_SAM_PORT_H
#define __ARCH_ARM_SRC_SAMV7_HARDWARE_SAM_PORT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* PORT Register Offsets ****************************************************/

#define SAM_PORT_DIR_OFFSET          0x0000 /* Data Direction Register */
#define SAM_PORT_DIRCLR_OFFSET       0x0004 /* Data Direction Clear Register */
#define SAM_PORT_DIRSET_OFFSET       0x0008 /* Data Direction Set Register */
#define SAM_PORT_DIRTGL_OFFSET       0x000c /* Data Direction Toggle Register */
#define SAM_PORT_OUT_OFFSET          0x0010 /* Data Output Value Register */
#define SAM_PORT_OUTCLR_OFFSET       0x0014 /* Data Output Value Clear Register */
#define SAM_PORT_OUTSET_OFFSET       0x0018 /* Data Output Value Set Register */
#define SAM_PORT_OUTTGL_OFFSET       0x001c /* Data Output Value Toggle Register */
#define SAM_PORT_IN_OFFSET           0x0020 /* Data Input Value Register */
#define SAM_PORT_CTRL_OFFSET         0x0024 /* Control Register */
#define SAM_PORT_WRCONFIG_OFFSET     0x0028 /* Write Configuration Register */
#define SAM_PORT_EVCTRL_OFFSET       0x002c /* Event Control Register */
#define SAM_PORT_PMUX_OFFSET(n)      (0x0030 + (n)) /* Peripheral Multiplexing Register n=0..15 */
#define SAM_PORT_PINCFG_OFFSET(n)    (0x0040 + (n)) /* Pin Configuration Register n=0..31 */

/* PORT Base Addresses ******************************************************/

#define SAM_PORT_BASE                0x44840000
#define SAM_PORT_GROUP_OFFSET(n)     ((n) << 7)
#define SAM_PORTN_BASE(n)            (SAM_PORT_BASE + SAM_PORT_GROUP_OFFSET(n))

#define SAM_PORTA_BASE               SAM_PORTN_BASE(0)
#define SAM_PORTB_BASE               SAM_PORTN_BASE(1)
#define SAM_PORTC_BASE               SAM_PORTN_BASE(2)
#define SAM_PORTD_BASE               SAM_PORTN_BASE(3)
#define SAM_PORTE_BASE               SAM_PORTN_BASE(4)
#define SAM_PORTF_BASE               SAM_PORTN_BASE(5)
#define SAM_PORTG_BASE               SAM_PORTN_BASE(6)

/* PORT Register Bit Definitions ********************************************/

/* Write Configuration Register */

#define PORT_WRCONFIG_PINMASK_SHIFT  (0)       /* Bits 0-15: Pin Mask */
#define PORT_WRCONFIG_PINMASK_MASK   (0xffff << PORT_WRCONFIG_PINMASK_SHIFT)
#define PORT_WRCONFIG_PMUXEN         (1 << 16) /* Bit 16: Peripheral Multiplexer Enable */
#define PORT_WRCONFIG_INEN           (1 << 17) /* Bit 17: Input Enable */
#define PORT_WRCONFIG_PULLEN         (1 << 18) /* Bit 18: Pull Enable */
#define PORT_WRCONFIG_DRVSTR         (1 << 22) /* Bit 22: Output Driver Strength Selection */
#define PORT_WRCONFIG_PMUX_SHIFT     (24)      /* Bits 24-27: Peripheral Multiplexing */
#define PORT_WRCONFIG_PMUX_MASK      (15 << PORT_WRCONFIG_PMUX_SHIFT)
#  define PORT_WRCONFIG_PMUX(n)      ((uint32_t)(n) << PORT_WRCONFIG_PMUX_SHIFT)
#define PORT_WRCONFIG_WRPMUX         (1 << 28) /* Bit 28: Write PMUX */
#define PORT_WRCONFIG_WRPINCFG       (1 << 30) /* Bit 30: Write PINCFG */
#define PORT_WRCONFIG_HWSEL          (1 << 31) /* Bit 31: Half-Word Select */

/* Peripheral Multiplexing Register */

#define PORT_PMUX_EVEN_SHIFT         (0)       /* Bits 0-3: Peripheral Multiplexing for Even-Numbered Pin */
#define PORT_PMUX_EVEN_MASK          (15 << PORT_PMUX_EVEN_SHIFT)
#  define PORT_PMUX_EVEN(n)          ((uint8_t)(n) << PORT_PMUX_EVEN_SHIFT)
#define PORT_PMUX_ODD_SHIFT          (4)       /* Bits 4-7: Peripheral Multiplexing for Odd-Numbered Pin */
#define PORT_PMUX_ODD_MASK           (15 << PORT_PMUX_ODD_SHIFT)
#  define PORT_PMUX_ODD(n)           ((uint8_t)(n) << PORT_PMUX_ODD_SHIFT)

/* Pin Configuration Register */

#define PORT_PINCFG_PMUXEN           (1 << 0)  /* Bit 0: Peripheral Multiplexer Enable */
#define PORT_PINCFG_INEN             (1 << 1)  /* Bit 1: Input Enable */
#define PORT_PINCFG_PULLEN           (1 << 2)  /* Bit 2: Pull Enable */
#define PORT_PINCFG_DRVSTR           (1 << 6)  /* Bit 6: Output Driver Strength Selection */

#endif /* __ARCH_ARM_SRC_SAMV7_HARDWARE_SAM_PORT_H */
