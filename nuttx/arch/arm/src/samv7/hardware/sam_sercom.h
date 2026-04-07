/****************************************************************************
 * arch/arm/src/samv7/hardware/sam_sercom.h
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

#ifndef __ARCH_ARM_SRC_SAMV7_HARDWARE_SAM_SERCOM_H
#define __ARCH_ARM_SRC_SAMV7_HARDWARE_SAM_SERCOM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SERCOM USART Register Offsets ********************************************/

#define SAM_SERCOM_CTRLA_OFFSET      0x0000 /* Control A Register */
#define SAM_SERCOM_CTRLB_OFFSET      0x0004 /* Control B Register */
#define SAM_SERCOM_CTRLC_OFFSET      0x0008 /* Control C Register */
#define SAM_SERCOM_BAUD_OFFSET       0x000c /* Baud Register */
#define SAM_SERCOM_RXERRCNT_OFFSET   0x0013 /* Receive Error Count Register */
#define SAM_SERCOM_INTENCLR_OFFSET   0x0014 /* Interrupt Enable Clear Register */
#define SAM_SERCOM_INTENSET_OFFSET   0x0016 /* Interrupt Enable Set Register */
#define SAM_SERCOM_INTFLAG_OFFSET    0x0018 /* Interrupt Flag Status and Clear Register */
#define SAM_SERCOM_STATUS_OFFSET     0x001a /* Status Register */
#define SAM_SERCOM_SYNCBUSY_OFFSET   0x001c /* Synchronization Busy Register */
#define SAM_SERCOM_RXCTRL_OFFSET     0x0020 /* Receive Control Register */
#define SAM_SERCOM_TXCTRL_OFFSET     0x0024 /* Transmit Control Register */
#define SAM_SERCOM_DATA_OFFSET       0x0028 /* Data Register */
#define SAM_SERCOM_DEBUGCTRL_OFFSET  0x0030 /* Debug Control Register */

/* SERCOM Base Addresses ****************************************************/

#define SAM_SERCOM0_BASE             0x46000000
#define SAM_SERCOM1_BASE             0x46002000
#define SAM_SERCOM2_BASE             0x46004000
#define SAM_SERCOM3_BASE             0x46006000
#define SAM_SERCOM4_BASE             0x46100000
#define SAM_SERCOM5_BASE             0x46102000
#define SAM_SERCOM6_BASE             0x46104000
#define SAM_SERCOM7_BASE             0x46106000

/* SERCOM USART Register Bit Definitions ************************************/

/* Control A Register */

#define SERCOM_USART_CTRLA_SWRST     (1 << 0)  /* Bit 0: Software Reset */
#define SERCOM_USART_CTRLA_ENABLE    (1 << 1)  /* Bit 1: Enable */
#define SERCOM_USART_CTRLA_MODE_SHIFT (2)      /* Bits 2-4: Operating Mode */
#define SERCOM_USART_CTRLA_MODE_MASK  (7 << SERCOM_USART_CTRLA_MODE_SHIFT)
#  define SERCOM_USART_CTRLA_MODE_USART_EXT_CLK (0 << SERCOM_USART_CTRLA_MODE_SHIFT)
#  define SERCOM_USART_CTRLA_MODE_USART_INT_CLK (1 << SERCOM_USART_CTRLA_MODE_SHIFT)
#define SERCOM_USART_CTRLA_RUNSTDBY  (1 << 7)  /* Bit 7: Run in Standby */
#define SERCOM_USART_CTRLA_IBON      (1 << 8)  /* Bit 8: Immediate Buffer Overflow Notification */
#define SERCOM_USART_CTRLA_TXPO_SHIFT (16)     /* Bits 16-17: Transmit Data Pinout */
#define SERCOM_USART_CTRLA_TXPO_MASK  (3 << SERCOM_USART_CTRLA_TXPO_SHIFT)
#  define SERCOM_USART_CTRLA_TXPO(n)  ((uint32_t)(n) << SERCOM_USART_CTRLA_TXPO_SHIFT)
#define SERCOM_USART_CTRLA_RXPO_SHIFT (20)     /* Bits 20-21: Receive Data Pinout */
#define SERCOM_USART_CTRLA_RXPO_MASK  (3 << SERCOM_USART_CTRLA_RXPO_SHIFT)
#  define SERCOM_USART_CTRLA_RXPO(n)  ((uint32_t)(n) << SERCOM_USART_CTRLA_RXPO_SHIFT)
#define SERCOM_USART_CTRLA_SAMPR_SHIFT (24)    /* Bits 24-26: Sample Rate */
#define SERCOM_USART_CTRLA_SAMPR_MASK  (7 << SERCOM_USART_CTRLA_SAMPR_SHIFT)
#  define SERCOM_USART_CTRLA_SAMPR_16X_ARITH (0 << SERCOM_USART_CTRLA_SAMPR_SHIFT)
#define SERCOM_USART_CTRLA_DORD      (1 << 30) /* Bit 30: Data Order */

/* Control B Register */

#define SERCOM_USART_CTRLB_CHSIZE_SHIFT (0)    /* Bits 0-2: Character Size */
#define SERCOM_USART_CTRLB_CHSIZE_MASK  (7 << SERCOM_USART_CTRLB_CHSIZE_SHIFT)
#  define SERCOM_USART_CTRLB_CHSIZE_8BITS (0 << SERCOM_USART_CTRLB_CHSIZE_SHIFT)
#define SERCOM_USART_CTRLB_SBMODE    (1 << 6)  /* Bit 6: Stop Bit Mode */
#define SERCOM_USART_CTRLB_COLDEN    (1 << 8)  /* Bit 8: Collision Detection Enable */
#define SERCOM_USART_CTRLB_SFDE      (1 << 9)  /* Bit 9: Start of Frame Detection Enable */
#define SERCOM_USART_CTRLB_ENC       (1 << 10) /* Bit 10: Encoding Format */
#define SERCOM_USART_CTRLB_PMODE     (1 << 13) /* Bit 13: Parity Mode */
#define SERCOM_USART_CTRLB_TXEN      (1 << 16) /* Bit 16: Transmitter Enable */
#define SERCOM_USART_CTRLB_RXEN      (1 << 17) /* Bit 17: Receiver Enable */

/* Interrupt Enable Clear/Set/Flag Registers */

#define SERCOM_USART_INT_DRE         (1 << 0)  /* Bit 0: Data Register Empty Interrupt */
#define SERCOM_USART_INT_TXC         (1 << 1)  /* Bit 1: Transmit Complete Interrupt */
#define SERCOM_USART_INT_RXC         (1 << 2)  /* Bit 2: Receive Complete Interrupt */
#define SERCOM_USART_INT_RXS         (1 << 3)  /* Bit 3: Receive Start Interrupt */
#define SERCOM_USART_INT_CTSIC       (1 << 4)  /* Bit 4: Clear to Send Input Change Interrupt */
#define SERCOM_USART_INT_RXBRK       (1 << 5)  /* Bit 5: Receive Break Interrupt */
#define SERCOM_USART_INT_ERROR       (1 << 7)  /* Bit 7: Combined Error Interrupt */

/* Synchronization Busy Register */

#define SERCOM_USART_SYNCBUSY_SWRST  (1 << 0)  /* Bit 0: Software Reset Synchronization Busy */
#define SERCOM_USART_SYNCBUSY_ENABLE (1 << 1)  /* Bit 1: Enable Synchronization Busy */
#define SERCOM_USART_SYNCBUSY_CTRLB  (1 << 2)  /* Bit 2: Control B Synchronization Busy */

#endif /* __ARCH_ARM_SRC_SAMV7_HARDWARE_SAM_SERCOM_H */
