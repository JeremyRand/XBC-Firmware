//==========================================================================
//
//      dev/if_quicc.c
//
//      Ethernet device driver for PowerPC QUICC (MPC8xx) boards
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  hardware driver for MPC8xx QUICC
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Ethernet device driver for MPC8xx QUICC

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_powerpc_quicc.h>
#include <pkgconf/io_eth_drivers.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#endif

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>

#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include "quicc_eth.h"

static unsigned char quicc_eth_rxbufs[CYGNUM_DEVS_ETH_POWERPC_QUICC_RxNUM]
                                     [CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE];
static unsigned char quicc_eth_txbufs[CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM]
                                     [CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE];

static struct quicc_eth_info quicc_eth0_info;
static unsigned char _default_enaddr[] = { 0x08, 0x00, 0x3E, 0x28, 0x79, 0xB8};
static unsigned char enaddr[6];
#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
RedBoot_config_option("Network hardware address [MAC]",
                      quicc_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
#endif
#endif

// For fetching the ESA from RedBoot
#include <cyg/hal/hal_if.h>
#ifndef CONFIG_ESA
#define CONFIG_ESA 6
#endif

ETH_DRV_SC(quicc_eth0_sc,
           &quicc_eth0_info,   // Driver specific data
           "eth0",             // Name for this interface
           quicc_eth_start,
           quicc_eth_stop,
           quicc_eth_control,
           quicc_eth_can_send,
           quicc_eth_send,
           quicc_eth_recv,
           quicc_eth_deliver,
           quicc_eth_int,
           quicc_eth_int_vector);

NETDEVTAB_ENTRY(quicc_netdev, 
                "quicc_eth", 
                quicc_eth_init, 
                &quicc_eth0_sc);

extern int _mbx_fetch_VPD(int, void *, int);

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
static cyg_interrupt quicc_eth_interrupt;
static cyg_handle_t  quicc_eth_interrupt_handle;
#endif
static void          quicc_eth_int(struct eth_drv_sc *data);

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
// This ISR is called when the ethernet interrupt occurs
static int
quicc_eth_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_CPM_SCC1);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_CPM_SCC1);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}
#endif

// Deliver function (ex-DSR) handles the ethernet [logical] processing
static void
quicc_eth_deliver(struct eth_drv_sc * sc)
{
    quicc_eth_int(sc);
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_CPM_SCC1);
#endif
}

//
// Initialize the interface - performed at system startup
// This function must set up the interface, including arranging to
// handle interrupts, etc, so that it may be "started" cheaply later.
//
static bool 
quicc_eth_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();
    struct cp_bufdesc *rxbd, *txbd;
    unsigned char *RxBUF, *TxBUF, *ep, *ap; 
    volatile struct ethernet_pram *enet_pram;
    volatile struct scc_regs *scc;
    int TxBD, RxBD;
    int cache_state;
    int i;
    bool esa_ok;

    // Fetch the board address from the VPD
#define VPD_ETHERNET_ADDRESS 0x08
    if (_mbx_fetch_VPD(VPD_ETHERNET_ADDRESS, enaddr, sizeof(enaddr)) == 0) {
#if defined(CYGPKG_REDBOOT) && \
    defined(CYGSEM_REDBOOT_FLASH_CONFIG)
        esa_ok = flash_get_config("quicc_esa", enaddr, CONFIG_ESA);
#else
        esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,         
                                             "quicc_esa", enaddr, CONFIG_ESA);
#endif
        if (!esa_ok) {
            // Can't figure out ESA
            diag_printf("QUICC_ETH - Warning! ESA unknown\n");
            memcpy(&enaddr, &_default_enaddr, sizeof(enaddr));
        }
    }

    // Ensure consistent state between cache and what the QUICC sees
    HAL_DCACHE_IS_ENABLED(cache_state);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    // Set up to handle interrupts
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_CPM_SCC1,
                             CYGARC_SIU_PRIORITY_HIGH,
                             (cyg_addrword_t)sc, //  Data item passed to interrupt handler
                             (cyg_ISR_t *)quicc_eth_isr,
                             (cyg_DSR_t *)eth_drv_dsr,
                             &quicc_eth_interrupt_handle,
                             &quicc_eth_interrupt);
    cyg_drv_interrupt_attach(quicc_eth_interrupt_handle);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_CPM_SCC1);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_CPM_SCC1);
#endif

    qi->pram = enet_pram = &eppc->pram[0].enet_scc;
    qi->ctl = scc = &eppc->scc_regs[0];  // Use SCC1

    // Shut down ethernet, in case it is already running
    scc->scc_gsmr_l &= ~(QUICC_SCC_GSML_ENR | QUICC_SCC_GSML_ENT);

    memset((void *)enet_pram, 0, sizeof(*enet_pram));

    TxBD = 0x2C00;  // FIXME
    RxBD = TxBD + CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM * sizeof(struct cp_bufdesc);

    txbd = (struct cp_bufdesc *)((char *)eppc + TxBD);
    rxbd = (struct cp_bufdesc *)((char *)eppc + RxBD);
    qi->tbase = txbd;
    qi->txbd = txbd;    
    qi->tnext = txbd;
    qi->rbase = rxbd;
    qi->rxbd = rxbd;
    qi->rnext = rxbd;

    RxBUF = &quicc_eth_rxbufs[0][0];
    TxBUF = &quicc_eth_txbufs[0][0];

    // setup buffer descriptors
    for (i = 0;  i < CYGNUM_DEVS_ETH_POWERPC_QUICC_RxNUM;  i++) {
        rxbd->length = 0;
        rxbd->buffer = RxBUF;
        rxbd->ctrl   = QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int;
        RxBUF += CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;
        rxbd++;
    }
    rxbd--;
    rxbd->ctrl |= QUICC_BD_CTL_Wrap;  // Last buffer
    for (i = 0;  i < CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM;  i++) {
        txbd->length = 0;
        txbd->buffer = TxBUF;
        txbd->ctrl   = 0;
        TxBUF += CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;
        txbd++;
    }
    txbd--;
    txbd->ctrl |= QUICC_BD_CTL_Wrap;  // Last buffer

    // Set up parallel ports for connection to MC68160 ethernet tranceiver
    eppc->pio_papar |= (QUICC_MBX_PA_RXD | QUICC_MBX_PA_TXD);
    eppc->pio_padir &= ~(QUICC_MBX_PA_RXD | QUICC_MBX_PA_TXD);
    eppc->pio_paodr &= ~QUICC_MBX_PA_TXD;

    eppc->pio_pcpar &= ~(QUICC_MBX_PC_COLLISION | QUICC_MBX_PC_Rx_ENABLE);
    eppc->pio_pcdir &= ~(QUICC_MBX_PC_COLLISION | QUICC_MBX_PC_Rx_ENABLE);
    eppc->pio_pcso  |= (QUICC_MBX_PC_COLLISION | QUICC_MBX_PC_Rx_ENABLE);

    eppc->pio_papar |= (QUICC_MBX_PA_Tx_CLOCK | QUICC_MBX_PA_Rx_CLOCK);
    eppc->pio_padir &= ~(QUICC_MBX_PA_Tx_CLOCK | QUICC_MBX_PA_Rx_CLOCK);

    // Set up clock routing
    eppc->si_sicr &= ~QUICC_MBX_SICR_MASK;
    eppc->si_sicr |= QUICC_MBX_SICR_ENET;
    eppc->si_sicr &= ~QUICC_MBX_SICR_SCC1_ENABLE;

    // Set up DMA mode
    eppc->dma_sdcr = 0x0001;

    // Initialize shared PRAM
    enet_pram->rbase = RxBD;
    enet_pram->tbase = TxBD;

    // Set Big Endian mode
    enet_pram->rfcr = QUICC_SCC_FCR_BE;
    enet_pram->tfcr = QUICC_SCC_FCR_BE;

    // Size of receive buffers
    enet_pram->mrblr = CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;

    // Initialize CRC calculations
    enet_pram->c_pres = 0xFFFFFFFF;
    enet_pram->c_mask = 0xDEBB20E3;  // Actual CRC formula
    enet_pram->crcec = 0;
    enet_pram->alec = 0;
    enet_pram->disfc = 0;

    // Frame padding
    enet_pram->pads = 0x8888;
    enet_pram->pads = 0x0000;

    // Retries
    enet_pram->ret_lim = 15;
    enet_pram->ret_cnt = 0;

    // Frame sizes
    enet_pram->mflr = IEEE_8023_MAX_FRAME;
    enet_pram->minflr = IEEE_8023_MIN_FRAME;
    enet_pram->maxd1 = CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;
    enet_pram->maxd2 = CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;

    // Group address hash
    enet_pram->gaddr1 = 0;
    enet_pram->gaddr2 = 0;
    enet_pram->gaddr3 = 0;
    enet_pram->gaddr4 = 0;

    // Device physical address
    ep = &enaddr[sizeof(enaddr)];
    ap = (unsigned char *)&enet_pram->paddr_h;
    for (i = 0;  i < sizeof(enaddr);  i++) {
        *ap++ = *--ep;
    }

    // Persistence counter
    enet_pram->p_per = 0; 

    // Individual address filter
    enet_pram->iaddr1 = 0;
    enet_pram->iaddr2 = 0;
    enet_pram->iaddr3 = 0;
    enet_pram->iaddr4 = 0;

    // Temp address
    enet_pram->taddr_h = 0;
    enet_pram->taddr_m = 0;
    enet_pram->taddr_l = 0;

    // Initialize the CPM (set up buffer pointers, etc).
    eppc->cp_cr = QUICC_CPM_SCC1 | QUICC_CPM_CR_INIT_TXRX | QUICC_CPM_CR_BUSY;
    while (eppc->cp_cr & QUICC_CPM_CR_BUSY) ;

    // Clear any pending interrupt/exceptions
    scc->scc_scce = 0xFFFF;

    // Enable interrupts
    scc->scc_sccm = QUICC_SCCE_INTS;

    // Set up SCC1 to run in ethernet mode
    scc->scc_gsmr_h = 0;
    scc->scc_gsmr_l = QUICC_SCC_GSML_TCI | QUICC_SCC_GSML_TPL_48 |
        QUICC_SCC_GSML_TPP_01 | QUICC_SCC_GSML_MODE_ENET;

    // Sync delimiters
    scc->scc_dsr = 0xD555;

    // Protocol specifics (as if GSML wasn't enough)
    scc->scc_psmr = QUICC_PMSR_ENET_CRC | QUICC_PMSR_SEARCH_AFTER_22 |
        QUICC_PMSR_RCV_SHORT_FRAMES;

    // Configure board interface
    *MBX_CTL1 = MBX_CTL1_ETEN | MBX_CTL1_TPEN;  // Enable ethernet, TP mode

    // Enable ethernet interface
    eppc->pio_pcpar |= QUICC_MBX_PC_Tx_ENABLE;
    eppc->pio_pcdir &= ~QUICC_MBX_PC_Tx_ENABLE;

    if (cache_state)
        HAL_DCACHE_ENABLE();

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, (unsigned char *)&enaddr);

    return true;
}

//
// This function is called to shut down the interface.
//
static void
quicc_eth_stop(struct eth_drv_sc *sc)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct scc_regs *scc = qi->ctl;

    // Disable the device!
    scc->scc_gsmr_l &= ~(QUICC_SCC_GSML_ENR | QUICC_SCC_GSML_ENT);
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
quicc_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct scc_regs *scc = qi->ctl;

    // Enable the device!
    scc->scc_gsmr_l |= QUICC_SCC_GSML_ENR | QUICC_SCC_GSML_ENT;
}

//
// This function is called for low level "control" operations
//
static int
quicc_eth_control(struct eth_drv_sc *sc, unsigned long key,
                  void *data, int length)
{
    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
        return 0;
        break;
    default:
        return 1;
        break;
    }
}

//
// This function is called to see if another packet can be sent.
// It should return the number of packets which can be handled.
// Zero should be returned if the interface is busy and can not send any more.
//
static int
quicc_eth_can_send(struct eth_drv_sc *sc)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct cp_bufdesc *txbd = qi->txbd;

    return ((txbd->ctrl & QUICC_BD_CTL_Ready) == 0);
}

//
// This routine is called to send data to the hardware.
static void 
quicc_eth_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
               int total_len, unsigned long key)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct cp_bufdesc *txbd, *txfirst;
    volatile char *bp;
    int i, txindex, cache_state;
    unsigned int ctrl;

    // Find a free buffer
    txbd = txfirst = qi->txbd;
    while (txbd->ctrl & QUICC_BD_CTL_Ready) {
        // This buffer is busy, move to next one
        if (txbd->ctrl & QUICC_BD_CTL_Wrap) {
            txbd = qi->tbase;
        } else {
            txbd++;
        }
        if (txbd == txfirst) {
#ifdef CYGPKG_NET
            panic ("No free xmit buffers");
#else
            diag_printf("QUICC Ethernet: No free xmit buffers\n");
#endif
        }
    }
    // Remember the next buffer to try
    if (txbd->ctrl & QUICC_BD_CTL_Wrap) {
        qi->txbd = qi->tbase;
    } else {
        qi->txbd = txbd+1;
    }
    txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
    qi->txkey[txindex] = key;
    // Set up buffer
    txbd->length = total_len;
    bp = txbd->buffer;
    for (i = 0;  i < sg_len;  i++) {
        memcpy((void *)bp, (void *)sg_list[i].buf, sg_list[i].len);
        bp += sg_list[i].len;
    }
    // Note: the MBX860 does not seem to snoop/invalidate the data cache properly!
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
        HAL_DCACHE_FLUSH(txbd->buffer, txbd->length);  // Make sure no stale data
    }
    // Send it on it's way
    ctrl = txbd->ctrl & ~QUICC_BD_TX_PAD;
    if (txbd->length < IEEE_8023_MIN_FRAME) {
        ctrl |= QUICC_BD_TX_PAD;
    }
    txbd->ctrl = ctrl | QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int | 
        QUICC_BD_TX_LAST | QUICC_BD_TX_TC;
}

//
// This function is called when a packet has been received.  It's job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'quicc_eth_recv' will be called to actually fetch it from the hardware.
//
static void
quicc_eth_RxEvent(struct eth_drv_sc *sc)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct cp_bufdesc *rxbd;

    rxbd = qi->rnext;
    while ((rxbd->ctrl & (QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int)) == QUICC_BD_CTL_Int) {
        qi->rxbd = rxbd;  // Save for callback
        (sc->funs->eth_drv->recv)(sc, rxbd->length);
        rxbd->ctrl |= QUICC_BD_CTL_Ready;
        if (rxbd->ctrl & QUICC_BD_CTL_Wrap) {
            rxbd = qi->rbase;
        } else {
            rxbd++;
        }
    }
    // Remember where we left off
    qi->rnext = (struct cp_bufdesc *)rxbd;
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// It's job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
quicc_eth_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    unsigned char *bp;
    int i, cache_state;

    bp = (unsigned char *)qi->rxbd->buffer;
    // Note: the MBX860 does not seem to snoop/invalidate the data cache properly!
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
        HAL_DCACHE_INVALIDATE(qi->rxbd->buffer, qi->rxbd->length);  // Make sure no stale data
    }
    for (i = 0;  i < sg_len;  i++) {
        if (sg_list[i].buf != 0) {
            memcpy((void *)sg_list[i].buf, bp, sg_list[i].len);
            bp += sg_list[i].len;
        }
    }
}

static void
quicc_eth_TxEvent(struct eth_drv_sc *sc, int stat)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct cp_bufdesc *txbd;
    int txindex;

    txbd = qi->tnext;
    while ((txbd->ctrl & (QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int)) == QUICC_BD_CTL_Int) {
        txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
        txbd->ctrl &= ~QUICC_BD_CTL_Int;  // Reset int pending bit
        (sc->funs->eth_drv->tx_done)(sc, qi->txkey[txindex], 0);
        if (txbd->ctrl & QUICC_BD_CTL_Wrap) {
            txbd = qi->tbase;
        } else {
            txbd++;
        }
    }
    // Remember where we left off
    qi->tnext = (struct cp_bufdesc *)txbd;
}

//
// Interrupt processing
//
static void          
quicc_eth_int(struct eth_drv_sc *sc)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct scc_regs *scc = qi->ctl;
    unsigned short scce;

    while ((scce = (scc->scc_scce & QUICC_SCCE_INTS)) != 0) {
        if ((scce & (QUICC_SCCE_TXE | QUICC_SCCE_TX)) != 0) {
            quicc_eth_TxEvent(sc, scce);
        }
        if ((scce & QUICC_SCCE_RXF) != 0) {
            quicc_eth_RxEvent(sc);
        }
        scc->scc_scce = scce;  // Reset the bits we handled
    }
}

//
// Interrupt vector
//
static int          
quicc_eth_int_vector(struct eth_drv_sc *sc)
{
    return (CYGNUM_HAL_INTERRUPT_CPM_SCC1);
}
