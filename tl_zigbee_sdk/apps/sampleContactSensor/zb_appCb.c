/********************************************************************************************************
 * @file    zb_appCb.c
 *
 * @brief   This is the source file for zb_appCb
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#if (__PROJECT_TL_CONTACT_SENSOR__)

/**********************************************************************
 * INCLUDES
 */
#include "tl_common.h"
#include "zb_api.h"
#include "zcl_include.h"
#include "bdb.h"
#include "ota.h"
#include "sampleSensor.h"
#include "app_ui.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */


/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * LOCAL FUNCTIONS
 */
void zbdemo_bdbInitCb(u8 status, u8 joinedNetwork);
void zbdemo_bdbCommissioningCb(u8 status, void *arg);
void zbdemo_bdbIdentifyCb(u8 endpoint, u16 srcAddr, u16 identifyTime);


/**********************************************************************
 * LOCAL VARIABLES
 */
bdb_appCb_t g_zbDemoBdbCb = {
    zbdemo_bdbInitCb,
    zbdemo_bdbCommissioningCb,
    zbdemo_bdbIdentifyCb,
    NULL
};

#ifdef ZCL_OTA
ota_callBack_t sampleSensor_otaCb = {
    sampleSensor_otaProcessMsgHandler,
};
#endif

ev_timer_event_t *steerTimerEvt = NULL;

/**********************************************************************
 * FUNCTIONS
 */
s32 sampleSensor_bdbNetworkSteerStart(void *arg)
{
    bdb_networkSteerStart();

    steerTimerEvt = NULL;
    return -1;
}

/*********************************************************************
 * @fn      zbdemo_bdbInitCb
 *
 * @brief   application callback for bdb initiation
 *
 * @param   status - the status of bdb init BDB_INIT_STATUS_SUCCESS or BDB_INIT_STATUS_FAILURE
 *
 * @param   joinedNetwork  - 1: node is on a network, 0: node isn't on a network
 *
 * @return  None
 */
void zbdemo_bdbInitCb(u8 status, u8 joinedNetwork)
{
    if (status == BDB_INIT_STATUS_SUCCESS) {
        /*
         * for non-factory-new device:
         * load zcl data from NV, start poll rate, start ota query, bdb_networkSteerStart
         *
         * for factory-new device:
         * steer a network
         *
         */
        if (joinedNetwork) {
            zb_setPollRate(POLL_RATE);

#ifdef ZCL_OTA
            ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
#endif

#ifdef ZCL_POLL_CTRL
            sampleSensor_zclCheckInStart();
#endif
        } else {
            u16 jitter = 0;
            do {
                jitter = zb_random() % 0x0fff;
            } while(jitter == 0);

            if (steerTimerEvt) {
                TL_ZB_TIMER_CANCEL(&steerTimerEvt);
            }
            steerTimerEvt = TL_ZB_TIMER_SCHEDULE(sampleSensor_bdbNetworkSteerStart, NULL, jitter);
        }
    } else {
        if (joinedNetwork) {
            zb_rejoinReqWithBackOff(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
        }
    }
}

/*********************************************************************
 * @fn      zbdemo_bdbCommissioningCb
 *
 * @brief   application callback for bdb commissioning
 *
 * @param   status - the status of bdb commissioning
 *
 * @param   arg
 *
 * @return  None
 */
void zbdemo_bdbCommissioningCb(u8 status, void *arg)
{
    switch (status) {
    case BDB_COMMISSION_STA_SUCCESS:
        light_blink_start(2, 200, 200);

        zb_setPollRate(POLL_RATE);

        if (steerTimerEvt) {
            TL_ZB_TIMER_CANCEL(&steerTimerEvt);
        }

#ifdef ZCL_POLL_CTRL
        sampleSensor_zclCheckInStart();
#endif
#ifdef ZCL_OTA
        ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
#endif
        break;
    case BDB_COMMISSION_STA_IN_PROGRESS:
        break;
    case BDB_COMMISSION_STA_NOT_AA_CAPABLE:
        break;
    case BDB_COMMISSION_STA_NO_NETWORK:
    case BDB_COMMISSION_STA_TCLK_EX_FAILURE:
    case BDB_COMMISSION_STA_TARGET_FAILURE:
        {
            u16 jitter = 0;
            do {
                jitter = zb_random() % 0x0fff;
            } while(jitter == 0);

            if (steerTimerEvt) {
                TL_ZB_TIMER_CANCEL(&steerTimerEvt);
            }
            steerTimerEvt = TL_ZB_TIMER_SCHEDULE(sampleSensor_bdbNetworkSteerStart, NULL, jitter);
        }
        break;
    case BDB_COMMISSION_STA_FORMATION_FAILURE:
        break;
    case BDB_COMMISSION_STA_NO_IDENTIFY_QUERY_RESPONSE:
        break;
    case BDB_COMMISSION_STA_BINDING_TABLE_FULL:
        break;
    case BDB_COMMISSION_STA_NO_SCAN_RESPONSE:
        break;
    case BDB_COMMISSION_STA_NOT_PERMITTED:
        break;
    case BDB_COMMISSION_STA_PARENT_LOST:
        //zb_rejoinSecModeSet(REJOIN_INSECURITY);
        zb_rejoinReq(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
        break;
    case BDB_COMMISSION_STA_REJOIN_FAILURE:
        break;
    default:
        break;
    }
}

extern void sampleSensor_zclIdentifyCmdHandler(u8 endpoint, u16 srcAddr, u16 identifyTime);
void zbdemo_bdbIdentifyCb(u8 endpoint, u16 srcAddr, u16 identifyTime)
{
    sampleSensor_zclIdentifyCmdHandler(endpoint, srcAddr, identifyTime);
}

#ifdef ZCL_OTA
void sampleSensor_otaProcessMsgHandler(u8 evt, u8 status)
{
    if (evt == OTA_EVT_START) {
        if (status == ZCL_STA_SUCCESS) {
            zb_setPollRate(QUEUE_POLL_RATE);
        } else {

        }
    } else if (evt == OTA_EVT_COMPLETE) {
        zb_setPollRate(POLL_RATE);

        if (status == ZCL_STA_SUCCESS) {
            ota_mcuReboot();
        } else {
            ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
        }
    } else if (evt == OTA_EVT_IMAGE_DONE) {
        zb_setPollRate(POLL_RATE);
    }
}
#endif

/*********************************************************************
 * @fn      sampleSensor_leaveCnfHandler
 *
 * @brief   Handler for ZDO Leave Confirm message.
 *
 * @param   pRsp - parameter of leave confirm
 *
 * @return  None
 */
void sampleSensor_leaveCnfHandler(nlme_leave_cnf_t *pLeaveCnf)
{
    if (pLeaveCnf->status == SUCCESS) {
    	//SYSTEM_RESET();
    }
}

/*********************************************************************
 * @fn      sampleSensor_leaveIndHandler
 *
 * @brief   Handler for ZDO leave indication message.
 *
 * @param   pInd - parameter of leave indication
 *
 * @return  None
 */
void sampleSensor_leaveIndHandler(nlme_leave_ind_t *pLeaveInd)
{
    //printf("sampleSensor_leaveIndHandler, rejoin = %d\n", pLeaveInd->rejoin);
    //printfArray(pLeaveInd->device_address, 8);
}

#endif  /* __PROJECT_TL_CONTACT_SENSOR__ */
