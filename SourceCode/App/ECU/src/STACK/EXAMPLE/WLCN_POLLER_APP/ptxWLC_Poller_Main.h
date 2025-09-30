/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : PTX130WN Demo
    File        : ptxWLC_Poller_Main.h

    Description : Wireless CHarging demo application for the PTX130W NFC Platform.

    The demo provides the user with the possibility to evaluate the PTX130W NFC
    Forum compliant Poller platform, running on a non-OS based host controller.
*/

#ifndef STACK_EXAMPLE_WLCN_POLLER_APP_PTXWLC_POLLER_MAIN_H_
#define STACK_EXAMPLE_WLCN_POLLER_APP_PTXWLC_POLLER_MAIN_H_

#include "ptxWLC_Poller_Main.h"
#include "ptxWLCN.h"
#include "ptxWLCNPeripherals.h"
#include "ptxWLCN_PollerDefines.h"
#include <string.h>

/**
 * \brief PTX130WN application.
 *
 * This function is the entry point for the PTX130WN application.
 *
 */
void ptx130WN_App(void);

#endif /* Guard */

