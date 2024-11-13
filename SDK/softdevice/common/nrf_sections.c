/**
 * Copyright (c) 2017 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"

extern void nrf_sdh_ble_evts_poll(void * p_context);
extern void nrf_sdh_soc_evts_poll(void * p_context);

// Create section set "sdh_ble_observers".
NRF_SECTION_SET_DEF(sdh_ble_observers, nrf_sdh_ble_evt_observer_t, NRF_SDH_BLE_OBSERVER_PRIO_LEVELS);

NRF_SDH_STACK_OBSERVER(m_nrf_sdh_ble_evts_poll, NRF_SDH_BLE_STACK_OBSERVER_PRIO) =
{
    .handler   = nrf_sdh_ble_evts_poll,
    .p_context = NULL,
};

// Create section set "sdh_soc_observers".
NRF_SECTION_SET_DEF(sdh_soc_observers, nrf_sdh_soc_evt_observer_t, NRF_SDH_SOC_OBSERVER_PRIO_LEVELS);

NRF_SDH_STACK_OBSERVER(m_nrf_sdh_soc_evts_poll, NRF_SDH_SOC_STACK_OBSERVER_PRIO) =
{
    .handler   = nrf_sdh_soc_evts_poll,
    .p_context = NULL,
};

