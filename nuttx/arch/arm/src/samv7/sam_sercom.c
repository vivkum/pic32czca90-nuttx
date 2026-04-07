/****************************************************************************
 * arch/arm/src/samv7/sam_sercom.c
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

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/serial/serial.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "hardware/sam_sercom.h"
#include "hardware/sam_port.h"

#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register access macros */

#define sercom_getreg8(p,o)      getreg8((p)->base + (o))
#define sercom_getreg16(p,o)     getreg16((p)->base + (o))
#define sercom_getreg32(p,o)     getreg32((p)->base + (o))

#define sercom_putreg8(v,p,o)    putreg8(v, (p)->base + (o))
#define sercom_putreg16(v,p,o)   putreg16(v, (p)->base + (o))
#define sercom_putreg32(v,p,o)   putreg32(v, (p)->base + (o))

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_sercom_s
{
  uintptr_t base;             /* Base address of SERCOM registers */
  uint32_t  baud;             /* Configured baud */
  uint8_t   irq;              /* IRQ associated with this SERCOM */
  uint8_t   id;               /* SERCOM ID (0-7) */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int  sercom_setup(struct uart_dev_s *dev);
static void sercom_shutdown(struct uart_dev_s *dev);
static int  sercom_attach(struct uart_dev_s *dev);
static void sercom_detach(struct uart_dev_s *dev);
static int  sercom_interrupt(int irq, void *context, void *arg);
static int  sercom_receive(struct uart_dev_s *dev, unsigned int *status);
static void sercom_rxint(struct uart_dev_s *dev, bool enable);
static bool sercom_rxavailable(struct uart_dev_s *dev);
static void sercom_send(struct uart_dev_s *dev, int ch);
static void sercom_txint(struct uart_dev_s *dev, bool enable);
static bool sercom_txready(struct uart_dev_s *dev);
static bool sercom_txempty(struct uart_dev_s *dev);

/****************************************************************************
 * Private Data
 ****************************************************************************/

#ifdef CONFIG_SAMV7_SERCOM1
static const struct uart_ops_s g_sercom_ops =
{
  .setup          = sercom_setup,
  .shutdown       = sercom_shutdown,
  .attach         = sercom_attach,
  .detach         = sercom_detach,
  .ioctl          = NULL,
  .receive        = sercom_receive,
  .rxint          = sercom_rxint,
  .rxavailable    = sercom_rxavailable,
#ifdef CONFIG_SERIAL_IFLOWCONTROL
  .rxflowcontrol  = NULL,
#endif
  .send           = sercom_send,
  .txint          = sercom_txint,
  .txready        = sercom_txready,
  .txempty        = sercom_txempty,
};

static struct sam_sercom_s g_sercom1priv =
{
  .base = SAM_SERCOM1_BASE,
  .baud = CONFIG_SERCOM1_BAUD,
  .irq  = 67, /* SERCOM1 IRQ on CA90 is 67 */
  .id   = 1,
};

static char g_sercom1rxbuffer[CONFIG_SERCOM1_RXBUFSIZE];
static char g_sercom1txbuffer[CONFIG_SERCOM1_TXBUFSIZE];

static struct uart_dev_s g_sercom1port =
{
  .recv     =
  {
    .size   = CONFIG_SERCOM1_RXBUFSIZE,
    .buffer = g_sercom1rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_SERCOM1_TXBUFSIZE,
    .buffer = g_sercom1txbuffer,
  },
  .ops      = &g_sercom_ops,
  .priv     = &g_sercom1priv,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void sercom_wait_sync(struct sam_sercom_s *priv, uint32_t mask)
{
  while ((sercom_getreg32(priv, SAM_SERCOM_SYNCBUSY_OFFSET) & mask) != 0);
}
#endif

static int sercom_setup(struct uart_dev_s *dev)
{
  /* Basic SERCOM1 setup is already done in sam_lowsetup() / sam_lowputc.c
   * for the console. Here we just ensure interrupts are configured.
   */

  return OK;
}

static void sercom_shutdown(struct uart_dev_s *dev)
{
  /* struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv; */
}

static int sercom_attach(struct uart_dev_s *dev)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;
  int ret;

  /* Attach and enable the IRQ */

  ret = irq_attach(priv->irq, sercom_interrupt, dev);
  if (ret == OK)
    {
      up_enable_irq(priv->irq);
    }

  return ret;
}

static void sercom_detach(struct uart_dev_s *dev)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;

  up_disable_irq(priv->irq);
  irq_detach(priv->irq);
}

static int sercom_interrupt(int irq, void *context, void *arg)
{
  struct uart_dev_s *dev = (struct uart_dev_s *)arg;
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;
  uint8_t intflag;
  uint8_t intenset;

  intflag = sercom_getreg8(priv, SAM_SERCOM_INTFLAG_OFFSET);
  intenset = sercom_getreg8(priv, SAM_SERCOM_INTENSET_OFFSET);

  /* Handle receive interrupts */

  if ((intflag & SERCOM_USART_INT_RXC) != 0 && (intenset & SERCOM_USART_INT_RXC) != 0)
    {
      uart_recvchars(dev);
    }

  /* Handle transmit interrupts */

  if ((intflag & SERCOM_USART_INT_DRE) != 0 && (intenset & SERCOM_USART_INT_DRE) != 0)
    {
      uart_xmitchars(dev);
    }

  return OK;
}

static int sercom_receive(struct uart_dev_s *dev, unsigned int *status)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;

  *status = (unsigned int)sercom_getreg16(priv, SAM_SERCOM_STATUS_OFFSET);
  return (int)sercom_getreg16(priv, SAM_SERCOM_DATA_OFFSET);
}

static void sercom_rxint(struct uart_dev_s *dev, bool enable)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;

  if (enable)
    {
      sercom_putreg8(SERCOM_USART_INT_RXC, priv, SAM_SERCOM_INTENSET_OFFSET);
    }
  else
    {
      sercom_putreg8(SERCOM_USART_INT_RXC, priv, SAM_SERCOM_INTENCLR_OFFSET);
    }
}

static bool sercom_rxavailable(struct uart_dev_s *dev)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;

  return (sercom_getreg8(priv, SAM_SERCOM_INTFLAG_OFFSET) & SERCOM_USART_INT_RXC) != 0;
}

static void sercom_send(struct uart_dev_s *dev, int ch)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;

  sercom_putreg16((uint16_t)ch, priv, SAM_SERCOM_DATA_OFFSET);
}

static void sercom_txint(struct uart_dev_s *dev, bool enable)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;

  if (enable)
    {
      sercom_putreg8(SERCOM_USART_INT_DRE, priv, SAM_SERCOM_INTENSET_OFFSET);
    }
  else
    {
      sercom_putreg8(SERCOM_USART_INT_DRE, priv, SAM_SERCOM_INTENCLR_OFFSET);
    }
}

static bool sercom_txready(struct uart_dev_s *dev)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;

  return (sercom_getreg8(priv, SAM_SERCOM_INTFLAG_OFFSET) & SERCOM_USART_INT_DRE) != 0;
}

static bool sercom_txempty(struct uart_dev_s *dev)
{
  struct sam_sercom_s *priv = (struct sam_sercom_s *)dev->priv;

  return (sercom_getreg8(priv, SAM_SERCOM_INTFLAG_OFFSET) & SERCOM_USART_INT_TXC) != 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_earlyserialinit
 *
 * Description:
 *   Performs the low level UART initialization early in debug so that the
 *   serial console will be available during bootup.  This must be called
 *   before sam_sercom_initialize.
 *
 ****************************************************************************/

void arm_earlyserialinit(void)
{
  /* The console SERCOM was already initialized in sam_lowsetup() */
}

/****************************************************************************
 * Name: arm_serialinit
 *
 * Description:
 *   Register all configured SERCOM USART devices.
 *
 ****************************************************************************/

void arm_serialinit(void)
{
#ifdef CONFIG_SAMV7_SERCOM1
  uart_register("/dev/ttyS0", &g_sercom1port);
#endif
}

#endif /* CONFIG_ARCH_CHIP_PIC32CZCA90 */
