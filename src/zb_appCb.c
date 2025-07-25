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

/**********************************************************************
 * INCLUDES
 */
#include "app_main.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */
#define DEBUG_HEART		0

/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * LOCAL FUNCTIONS
 */
void zb_bdbInitCb(uint8_t status, uint8_t joinedNetwork);
void zb_bdbCommissioningCb(uint8_t status, void *arg);
void zb_bdbIdentifyCb(uint8_t endpoint, uint16_t srcAddr, uint16_t identifyTime);


/**********************************************************************
 * GLOBAL VARIABLES
 */
bdb_appCb_t g_zbBdbCb = {
    zb_bdbInitCb,
    zb_bdbCommissioningCb,
    zb_bdbIdentifyCb,
    NULL
};

#ifdef ZCL_OTA
ota_callBack_t app_otaCb =
{
        app_otaProcessMsgHandler,
};
#endif

/**********************************************************************
 * LOCAL VARIABLES
 */
uint32_t heartInterval = 0;

#if DEBUG_HEART
ev_timer_event_t *heartTimerEvt = NULL;
#endif

/**********************************************************************
 * FUNCTIONS
 */
#if DEBUG_HEART
static int32_t heartTimerCb(void *arg){
	if(heartInterval == 0){
		heartTimerEvt = NULL;
		return -1;
	}

	gpio_toggle(LED_POWER);

	return heartInterval;
}
#endif

int32_t app_bdbNetworkSteerStart(void *arg){
	bdb_networkSteerStart();

	return -1;
}

#if FIND_AND_BIND_SUPPORT
int32_t app_bdbFindAndBindStart(void *arg){
	bdb_findAndBindStart(BDB_COMMISSIONING_ROLE_TARGET);

	return -1;
}
#endif

/*********************************************************************
 * @fn      zb_bdbInitCb
 *
 * @brief   application callback for bdb initiation
 *
 * @param   status - the status of bdb init BDB_INIT_STATUS_SUCCESS or BDB_INIT_STATUS_FAILURE
 *
 * @param   joinedNetwork  - 1: node is on a network, 0: node isn't on a network
 *
 * @return  None
 */
void zb_bdbInitCb(uint8_t status, uint8_t joinedNetwork){
//	printf("bdbInitCb: sta = %x, joined = %x\n", status, joinedNetwork);

	if(status == BDB_INIT_STATUS_SUCCESS){
		/*
		 * start bdb commissioning
		 * */
		if(joinedNetwork){
			heartInterval = 1000;

			device_online = true;
            g_appCtx.net_steer_start = false;

#ifdef ZCL_OTA
			ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
#endif
		} else if (g_appCtx.net_steer_start) {
			heartInterval = 500;

            device_online = false;

#if	(!ZBHCI_EN)
			uint16_t jitter = 0;
			do{
				jitter = zb_random() % 0x0fff;
			}while(jitter == 0);
			TL_ZB_TIMER_SCHEDULE(app_bdbNetworkSteerStart, NULL, jitter);
#endif
		}
	}else{
		heartInterval = 200;
	}

#if DEBUG_HEART
	if(heartTimerEvt){
		TL_ZB_TIMER_CANCEL(&heartTimerEvt);
	}
	heartTimerEvt = TL_ZB_TIMER_SCHEDULE(heartTimerCb, NULL, heartInterval);
#endif
}

/*********************************************************************
 * @fn      zb_bdbCommissioningCb
 *
 * @brief   application callback for bdb commissioning
 *
 * @param   status - the status of bdb commissioning
 *
 * @param   arg
 *
 * @return  None
 */
void zb_bdbCommissioningCb(uint8_t status, void *arg){
	//printf("bdbCommCb: sta = %x\n", status);

	switch(status){
		case BDB_COMMISSION_STA_SUCCESS:
			heartInterval = 1000;

			light_blink_start(2, 200, 200);

            device_online = true;
            g_appCtx.net_steer_start = false;

#ifdef ZCL_OTA
	    	ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
#endif

#if FIND_AND_BIND_SUPPORT
			if(!gLightCtx.bdbFindBindFlg){
				gLightCtx.bdbFindBindFlg = TRUE;
				TL_ZB_TIMER_SCHEDULE(app_bdbFindAndBindStart, NULL, 1000);
			}
#endif
			break;
		case BDB_COMMISSION_STA_IN_PROGRESS:
			break;
		case BDB_COMMISSION_STA_NOT_AA_CAPABLE:
			break;
		case BDB_COMMISSION_STA_NO_NETWORK:
		case BDB_COMMISSION_STA_TCLK_EX_FAILURE:
		case BDB_COMMISSION_STA_TARGET_FAILURE:
		    if (g_appCtx.net_steer_start) {
		        uint16_t jitter = 0;
	            device_online = false;
				do{
					jitter = zb_random() % 0x2710;
				}while(jitter < 5000);
				TL_ZB_TIMER_SCHEDULE(app_bdbNetworkSteerStart, NULL, jitter);
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
		case BDB_COMMISSION_STA_REJOIN_FAILURE:
			zb_rejoinReq(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
            device_online = false;
			break;
		case BDB_COMMISSION_STA_FORMATION_DONE:
#ifndef ZBHCI_EN
			tl_zbMacChannelSet(DEFAULT_CHANNEL);  //set default channel
#endif
			break;
		default:
			break;
	}
}


extern void app_zclIdentifyCmdHandler(uint8_t endpoint, uint16_t srcAddr, uint16_t identifyTime);
void zb_bdbIdentifyCb(uint8_t endpoint, uint16_t srcAddr, uint16_t identifyTime){
#if FIND_AND_BIND_SUPPORT
	app_zclIdentifyCmdHandler(endpoint, srcAddr, identifyTime);
#endif
}



#ifdef ZCL_OTA
void app_otaProcessMsgHandler(uint8_t evt, uint8_t status) {

	if(evt == OTA_EVT_START){
		if(status == ZCL_STA_SUCCESS){
#if UART_PRINTF_MODE && DEBUG_OTA
            printf("OTA update start.\r\n");
#endif /* UART_PRINTF_MODE */

//            dev_config.new_ota = true;
            write_config();

		}else{

		}
	}else if(evt == OTA_EVT_COMPLETE){
		if(status == ZCL_STA_SUCCESS){
#if UART_PRINTF_MODE && DEBUG_OTA
            printf("OTA update successful.\r\n");
#endif /* UART_PRINTF_MODE */
			ota_mcuReboot();
		}else{
#if UART_PRINTF_MODE && DEBUG_OTA
            printf("OTA update failure.\r\n");
#endif /* UART_PRINTF_MODE */
			ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
		}
	}
}

//extern ota_clientInfo_t otaClientInfo;
//
//void app_otaProcessMsgHandler(uint8_t evt, uint8_t status) {
//    printf("app_otaProcessMsgHandler: status = %x\r\n", status);
//    if (evt == OTA_EVT_START) {
//        if (status == ZCL_STA_SUCCESS) {
//
//#if UART_PRINTF_MODE && DEBUG_OTA
//            printf("OTA update start.\r\n");
//#endif /* UART_PRINTF_MODE */
//
//            dev_config.new_ota = true;
//            write_config();
//
//        } else {
//
//        }
//    } else if (evt == OTA_EVT_COMPLETE) {
//
//        if (status == ZCL_STA_SUCCESS) {
//
//#if UART_PRINTF_MODE && DEBUG_OTA
//            printf("OTA update successful.\r\n");
//#endif /* UART_PRINTF_MODE */
//
//            ota_mcuReboot();
//
//        } else {
//
//#if UART_PRINTF_MODE && DEBUG_OTA
//            printf("OTA update failure. Try again.\r\n");
//#endif /* UART_PRINTF_MODE */
//
//            /* reset update OTA */
//            nv_resetModule(NV_MODULE_OTA);
//
//            memset((uint8_t*) &otaClientInfo, 0, sizeof(otaClientInfo));
//            otaClientInfo.clientOtaFlg = OTA_FLAG_INIT_DONE;
//            otaClientInfo.crcValue = 0xffffffff;
//
//            zcl_attr_imageTypeID = 0xffff;
//            zcl_attr_fileOffset = 0xffffffff;
//            zcl_attr_downloadFileVer = 0xffffffff;
//
//            /* restore config */
//            init_config(false);
//
//            ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
//        }
//    }
//}

#endif

int32_t app_softReset(void *arg){
	SYSTEM_RESET();

	return -1;
}

/*********************************************************************
 * @fn      app_leaveCnfHandler
 *
 * @brief   Handler for ZDO Leave Confirm message.
 *
 * @param   pRsp - parameter of leave confirm
 *
 * @return  None
 */
void app_leaveCnfHandler(nlme_leave_cnf_t *pLeaveCnf)
{
    if(pLeaveCnf->status == SUCCESS){
    	light_blink_start(3, 200, 200);

    	//waiting blink over
    	TL_ZB_TIMER_SCHEDULE(app_softReset, NULL, 2 * 1000);
    }
}

/*********************************************************************
 * @fn      app_leaveIndHandler
 *
 * @brief   Handler for ZDO leave indication message.
 *
 * @param   pInd - parameter of leave indication
 *
 * @return  None
 */
void app_leaveIndHandler(nlme_leave_ind_t *pLeaveInd)
{

}

bool app_nwkUpdateIndicateHandler(nwkCmd_nwkUpdate_t *pNwkUpdate){
	return FAILURE;
}
