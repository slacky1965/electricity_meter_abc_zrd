/********************************************************************************************************
 * @file    zcl_reporting.c
 *
 * @brief   This is the source file for zcl_reporting
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
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
#include "zcl_include.h"

#define BUILD_U48(b0, b1, b2, b3, b4, b5)   ( (uint64_t)((((uint64_t)(b5) & 0x0000000000ff) << 40) + (((uint64_t)(b4) & 0x0000000000ff) << 32) + (((uint64_t)(b3) & 0x0000000000ff) << 24) + (((uint64_t)(b2) & 0x0000000000ff) << 16) + (((uint64_t)(b1) & 0x0000000000ff) << 8) + ((uint64_t)(b0) & 0x00000000FF)) )

/**********************************************************************
 * GLOBAL VARIABLES
 */
zcl_reportingTab_t reportingTab;

/**********************************************************************
 * LOCAL VARIABLES
 */
ev_timer_event_t *reportAttrTimerEvt = NULL;
static uint8_t counter_delivered_multdiv = 0;
static uint8_t counter_current_multdiv = 0;
static uint8_t counter_power_multdiv = 0;
static uint8_t counter_voltage_multdiv = 0;

/**********************************************************************
 * FUNCTIONS
 */

static void report_divisor_multiplier(uint8_t endpoint, uint16_t clusterID, uint16_t profileID, zclReportCmd_t *report) {

    //force report for multiplier and divisor
    //printf("report_divisor_multiplier. clusterId: 0x%x, attrId: 0x%x\r\n", pEntry->clusterID, pEntry->attrID);

    epInfo_t dstEpInfo;

    struct report_t {
        u8 numAttr;
        zclReport_t attr[2];
    };

    struct report_t muldiv_report;
    zclAttrInfo_t *pAttrEntry = NULL;

    for (uint8_t i = 0; i < report->numAttr; i++) {

        TL_SETSTRUCTCONTENT(dstEpInfo, 0);
        dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;
        dstEpInfo.profileId = profileID;

        if (clusterID == ZCL_CLUSTER_SE_METERING) {
            switch (report->attrList[i].attrID) {
                case ZCL_ATTRID_CURRENT_TIER_1_SUMMATION_DELIVERD:
                case ZCL_ATTRID_CURRENT_TIER_2_SUMMATION_DELIVERD:
                case ZCL_ATTRID_CURRENT_TIER_3_SUMMATION_DELIVERD:
                case ZCL_ATTRID_CURRENT_TIER_4_SUMMATION_DELIVERD:
                    if(counter_delivered_multdiv++ == 0) {
                        TL_SETSTRUCTCONTENT(muldiv_report, 0);
                        pAttrEntry = zcl_findAttribute(endpoint, clusterID, ZCL_ATTRID_MULTIPLIER);
                        if (pAttrEntry) {
                            muldiv_report.attr[muldiv_report.numAttr].attrData = pAttrEntry->data;
                            muldiv_report.attr[muldiv_report.numAttr].attrID = pAttrEntry->id;
                            muldiv_report.attr[muldiv_report.numAttr].dataType = pAttrEntry->type;
                            muldiv_report.numAttr++;
                        }
                        pAttrEntry = zcl_findAttribute(endpoint, clusterID, ZCL_ATTRID_DIVISOR);
                        if (pAttrEntry) {
                            muldiv_report.attr[muldiv_report.numAttr].attrData = pAttrEntry->data;
                            muldiv_report.attr[muldiv_report.numAttr].attrID = pAttrEntry->id;
                            muldiv_report.attr[muldiv_report.numAttr].dataType = pAttrEntry->type;
                            muldiv_report.numAttr++;
                        }
                        if (muldiv_report.numAttr) {
//                            printf("report tariff, counter_delivered_multdiv: %d\r\n", counter_delivered_multdiv);
                            zcl_sendReportAttrsCmd(endpoint, &dstEpInfo, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR, clusterID, (zclReportCmd_t* )&muldiv_report);
                        }
                    } else {
                        if (counter_delivered_multdiv == 10) {
                            counter_delivered_multdiv = 0;
                        }
                    }
                    break;
                default:
                    break;
            }
        } else if (clusterID == ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT) {
            switch (report->attrList[i].attrID) {
                case ZCL_ATTRID_RMS_CURRENT:
                    if(counter_current_multdiv++ == 0) {
                        TL_SETSTRUCTCONTENT(muldiv_report, 0);
                        pAttrEntry = zcl_findAttribute(endpoint, clusterID, ZCL_ATTRID_AC_CURRENT_MULTIPLIER);
                        if (pAttrEntry) {
                            muldiv_report.attr[muldiv_report.numAttr].attrData = pAttrEntry->data;
                            muldiv_report.attr[muldiv_report.numAttr].attrID = pAttrEntry->id;
                            muldiv_report.attr[muldiv_report.numAttr].dataType = pAttrEntry->type;
                            muldiv_report.numAttr++;
                        }
                        pAttrEntry = zcl_findAttribute(endpoint, clusterID, ZCL_ATTRID_AC_CURRENT_DIVISOR);
                        if (pAttrEntry) {
                            muldiv_report.attr[muldiv_report.numAttr].attrData = pAttrEntry->data;
                            muldiv_report.attr[muldiv_report.numAttr].attrID = pAttrEntry->id;
                            muldiv_report.attr[muldiv_report.numAttr].dataType = pAttrEntry->type;
                            muldiv_report.numAttr++;
                        }
                        if (muldiv_report.numAttr) {
//                            printf("report current, counter_current_multdiv: %d\r\n", counter_current_multdiv);
                            zcl_sendReportAttrsCmd(endpoint, &dstEpInfo, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR, clusterID, (zclReportCmd_t* )&muldiv_report);
                        }
                    } else {
                        if (counter_current_multdiv == 10) {
                            counter_current_multdiv = 0;
                        }
                    }
                    break;
                case ZCL_ATTRID_RMS_VOLTAGE:
                    if(counter_voltage_multdiv++ == 0) {
                        TL_SETSTRUCTCONTENT(muldiv_report, 0);
                        pAttrEntry = zcl_findAttribute(endpoint, clusterID, ZCL_ATTRID_AC_VOLTAGE_MULTIPLIER);
                        if (pAttrEntry) {
                            muldiv_report.attr[muldiv_report.numAttr].attrData = pAttrEntry->data;
                            muldiv_report.attr[muldiv_report.numAttr].attrID = pAttrEntry->id;
                            muldiv_report.attr[muldiv_report.numAttr].dataType = pAttrEntry->type;
                            muldiv_report.numAttr++;
                        }
                        pAttrEntry = zcl_findAttribute(endpoint, clusterID, ZCL_ATTRID_AC_VOLTAGE_DIVISOR);
                        if (pAttrEntry) {
                            muldiv_report.attr[muldiv_report.numAttr].attrData = pAttrEntry->data;
                            muldiv_report.attr[muldiv_report.numAttr].attrID = pAttrEntry->id;
                            muldiv_report.attr[muldiv_report.numAttr].dataType = pAttrEntry->type;
                            muldiv_report.numAttr++;
                        }
                        if (muldiv_report.numAttr) {
//                            printf("report voltage, counter_voltage_multdiv: %d\r\n", counter_voltage_multdiv);
                            zcl_sendReportAttrsCmd(endpoint, &dstEpInfo, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR, clusterID, (zclReportCmd_t* )&muldiv_report);
                        }
                    } else {
                        if (counter_voltage_multdiv == 10) {
                            counter_voltage_multdiv = 0;
                        }
                    }
                    break;
                case ZCL_ATTRID_ACTIVE_POWER:
                    if(counter_power_multdiv++ == 0) {
                        TL_SETSTRUCTCONTENT(muldiv_report, 0);
                        pAttrEntry = zcl_findAttribute(endpoint, clusterID, ZCL_ATTRID_AC_POWER_MULTIPLIER);
                        if (pAttrEntry) {
                            muldiv_report.attr[muldiv_report.numAttr].attrData = pAttrEntry->data;
                            muldiv_report.attr[muldiv_report.numAttr].attrID = pAttrEntry->id;
                            muldiv_report.attr[muldiv_report.numAttr].dataType = pAttrEntry->type;
                            muldiv_report.numAttr++;
                        }
                        pAttrEntry = zcl_findAttribute(endpoint, clusterID, ZCL_ATTRID_AC_POWER_DIVISOR);
                        if (pAttrEntry) {
                            muldiv_report.attr[muldiv_report.numAttr].attrData = pAttrEntry->data;
                            muldiv_report.attr[muldiv_report.numAttr].attrID = pAttrEntry->id;
                            muldiv_report.attr[muldiv_report.numAttr].dataType = pAttrEntry->type;
                            muldiv_report.numAttr++;
                        }
                        if (muldiv_report.numAttr) {
//                            printf("report power, counter_power_multdiv: %d\r\n", counter_power_multdiv);
                            zcl_sendReportAttrsCmd(endpoint, &dstEpInfo, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR, clusterID, (zclReportCmd_t* )&muldiv_report);
                        }
                    } else {
                        if (counter_power_multdiv == 10) {
                            counter_power_multdiv = 0;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}



/*********************************************************************
 * @fn      zcl_reportCfgInfoEntryClear
 *
 * @brief
 *
 * @param   pEntry
 *
 * @return	NULL
 */
_CODE_ZCL_ void zcl_reportCfgInfoEntryClear(reportCfgInfo_t *pEntry)
{
	if(pEntry){
		memset((u8 *)pEntry, 0, sizeof(reportCfgInfo_t));
	}
}

/*********************************************************************
 * @fn      zcl_reportCfgInfoEntryClear
 *
 * @brief
 *
 * @param   pEntry
 *
 * @return	NULL
 */
_CODE_ZCL_ void zcl_reportingTabInit(void)
{
	if(zcl_reportingTab_restore() != NV_SUCC){
		reportingTab.reportNum = 0;

		for(u8 i = 0; i < ZCL_REPORTING_TABLE_NUM; i++){
			reportCfgInfo_t *pEntry = &reportingTab.reportCfgInfo[i];

			zcl_reportCfgInfoEntryClear(pEntry);
		}
	}
}

/*********************************************************************
 * @fn      zcl_reportingEntryActiveNumGet
 *
 * @brief
 *
 * @param   NULL
 *
 * @return	Number of active reporting table
 */
_CODE_ZCL_ u8 zcl_reportingEntryActiveNumGet(void)
{
	u8 cnt = 0;

	if(reportingTab.reportNum){
		for(u8 i = 0; i < ZCL_REPORTING_TABLE_NUM; i++){
			if(reportingTab.reportCfgInfo[i].used && (reportingTab.reportCfgInfo[i].maxInterval != 0xFFFF)){
				cnt++;
			}
		}
	}

	return cnt;
}

/*********************************************************************
 * @fn      zcl_reportCfgInfoEntryFind
 *
 * @brief
 *
 * @param   clusterId
 * 			attrID
 *
 * @return	Reporting table entry
 */
_CODE_ZCL_ reportCfgInfo_t *zcl_reportCfgInfoEntryFind(u8 endpoint, u16 clusterId, u16 attrID)
{
	for(u8 i = 0; i < ZCL_REPORTING_TABLE_NUM; i++){
		if(reportingTab.reportCfgInfo[i].used){
			if( (reportingTab.reportCfgInfo[i].clusterID == clusterId) &&
				(reportingTab.reportCfgInfo[i].attrID == attrID) &&
				(reportingTab.reportCfgInfo[i].endPoint == endpoint) ){
					return &reportingTab.reportCfgInfo[i];
			}
		}
	}

	return NULL;
}

/*********************************************************************
 * @fn      zcl_reportCfgInfoEntryFreeGet
 *
 * @brief
 *
 * @param   NULL
 *
 * @return	Reporting table entry
 */
_CODE_ZCL_ reportCfgInfo_t *zcl_reportCfgInfoEntryFreeGet(void)
{
	for(u8 i = 0; i < ZCL_REPORTING_TABLE_NUM; i++){
		if(reportingTab.reportCfgInfo[i].used == 0){
			return &reportingTab.reportCfgInfo[i];
		}
	}

	return NULL;
}

/*********************************************************************
 * @fn      zcl_reportCfgInfoEntryRst2Default
 *
 * @brief
 *
 * @param   pEntry
 *
 * @return	NULL
 */
_CODE_ZCL_ void zcl_reportCfgInfoEntryRst(reportCfgInfo_t *pEntry)
{
	if(pEntry->used){
		pEntry->minInterval = pEntry->minIntDft;
		pEntry->maxInterval = pEntry->maxIntDft;
		pEntry->minIntCnt = pEntry->minIntDft;
		pEntry->maxIntCnt = pEntry->maxIntDft;
		memset(pEntry->reportableChange, 0, REPORTABLE_CHANGE_MAX_ANALOG_SIZE);

		reportAttrTimerStop();
		reportAttrTimerStart();
	}
}

/*********************************************************************
 * @fn      zcl_reportCfgInfoEntryUpdate
 *
 * @brief
 *
 * @param   pEntry
 * 			endPoint
 * 			profileId
 * 			clusterId
 * 			pCfgReportRec
 *
 * @return	NULL
 */
_CODE_ZCL_ void zcl_reportCfgInfoEntryUpdate(reportCfgInfo_t *pEntry, u8 endPoint, u16 profileId, u16 clusterId, zclCfgReportRec_t *pCfgReportRec)
{
	if(!pEntry->used){
		//add new
		pEntry->profileID = profileId;
		pEntry->clusterID = clusterId;
		pEntry->endPoint = endPoint;
		pEntry->attrID = pCfgReportRec->attrID;
		pEntry->minIntDft = pCfgReportRec->minReportInt;
		pEntry->maxIntDft = pCfgReportRec->maxReportInt;
		pEntry->minInterval = pCfgReportRec->minReportInt;
		pEntry->maxInterval = pCfgReportRec->maxReportInt;
		pEntry->dataType = pCfgReportRec->dataType;
		if(zcl_analogDataType(pEntry->dataType)){
			memcpy(pEntry->reportableChange, pCfgReportRec->reportableChange, zcl_getDataTypeLen(pEntry->dataType));
		}

		zclAttrInfo_t *pAttrEntry = zcl_findAttribute(pEntry->endPoint, pEntry->clusterID, pEntry->attrID);
		if(!pAttrEntry){
			//should not happen.
			ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_ZCL_ENTRY);
		}
		//u8 len = zcl_getAttrSize(pAttrEntry->type, pAttrEntry->data);
		//memcpy(pEntry->prevData, pAttrEntry->data, len);

		pEntry->minIntCnt = pCfgReportRec->minReportInt;
		pEntry->maxIntCnt = pCfgReportRec->maxReportInt;
		pEntry->used = 1;

		reportingTab.reportNum++;
	}else{
		//update
		pEntry->minInterval = pCfgReportRec->minReportInt;
		pEntry->maxInterval = pCfgReportRec->maxReportInt;
		pEntry->minIntCnt = pCfgReportRec->minReportInt;
		pEntry->maxIntCnt = pCfgReportRec->maxReportInt;
		if(zcl_analogDataType(pEntry->dataType)){
			memcpy(pEntry->reportableChange, pCfgReportRec->reportableChange, zcl_getDataTypeLen(pEntry->dataType));
		}
	}
}

/*********************************************************************
 * @fn      reportableChangeValueChk
 *
 * @brief
 *
 * @param   dataType
 * 			curValue
 * 			prevValue
 * 			reportableChange
 *
 * @return	TRUE / FALSE
 */
_CODE_ZCL_ bool reportableChangeValueChk(u8 dataType, u8 *curValue, u8 *prevValue, u8 *reportableChange)
{
	bool needReport = FALSE;

	switch(dataType)
	{
		case ZCL_DATA_TYPE_UINT8:
			{
				u8 P = prevValue[0];
				u8 C = curValue[0];
				u8 R = reportableChange[0];
				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
		case ZCL_DATA_TYPE_UINT16:
			{
				u16 P = BUILD_U16(prevValue[0], prevValue[1]);
				u16 C = BUILD_U16(curValue[0], curValue[1]);
				u16 R = BUILD_U16(reportableChange[0], reportableChange[1]);

				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
		case ZCL_DATA_TYPE_UINT24:
			{
				u24 P = BUILD_U24(prevValue[0], prevValue[1], prevValue[2]);//*((u24 *)prevValue);
				u24 C = BUILD_U24(curValue[0], curValue[1], curValue[2]);//*((u24 *)curValue);
				u24 R = BUILD_U24(reportableChange[0], reportableChange[1], reportableChange[2]);//*((u24 *)reportableChange);
				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
		case ZCL_DATA_TYPE_UINT32:
			{
				u32 P = BUILD_U32(prevValue[0], prevValue[1], prevValue[2], prevValue[3]);//*((u32 *)prevValue);
				u32 C = BUILD_U32(curValue[0], curValue[1], curValue[2], curValue[3]);//*((u32 *)curValue);
				u32 R = BUILD_U32(reportableChange[0], reportableChange[1], reportableChange[2], reportableChange[3]);//*((u32 *)reportableChange);
				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
        case ZCL_DATA_TYPE_UINT48:
            {
                uint64_t P = BUILD_U48(prevValue[0], prevValue[1], prevValue[2], prevValue[3], prevValue[4], prevValue[5]);
                uint64_t C = BUILD_U48(curValue[0], curValue[1], curValue[2], curValue[3], curValue[4], curValue[5]);
                uint64_t R = BUILD_U48(reportableChange[0], reportableChange[1], reportableChange[2], reportableChange[3], reportableChange[4], reportableChange[5]);
                if(P > C){
                    needReport = ((P - C) >= R) ? TRUE : FALSE;
                }else if(P < C){
                    needReport = ((C - P) >= R) ? TRUE : FALSE;
                }
//                printf("P: %s, need: %d\r\n", digit64toString(P), needReport);
//                printf("C: %s\r\n", digit64toString(C));
//                printf("R: %s\r\n", digit64toString(R));
            }
            break;
		case ZCL_DATA_TYPE_INT8:
			{
				s8 P = (s8)prevValue[0];
				s8 C = (s8)curValue[0];
				s8 R = (s8)reportableChange[0];
				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
		case ZCL_DATA_TYPE_INT16:
			{
				s16 P = BUILD_S16(prevValue[0], prevValue[1]);
				s16 C = BUILD_S16(curValue[0], curValue[1]);
				s16 R = BUILD_S16(reportableChange[0], reportableChange[1]);

				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
		case ZCL_DATA_TYPE_INT24:
			{
				s24 P = BUILD_S24(prevValue[0], prevValue[1], prevValue[2]);//*((s24 *)prevValue);
				s24 C = BUILD_S24(curValue[0], curValue[1], curValue[2]);//*((s24 *)curValue);
				s24 R = BUILD_S24(reportableChange[0], reportableChange[1], reportableChange[2]);//*((s24 *)reportableChange);
				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
		case ZCL_DATA_TYPE_INT32:
			{
				s32 P = BUILD_S32(prevValue[0], prevValue[1], prevValue[2], prevValue[3]);//*((s32 *)prevValue);
				s32 C = BUILD_S32(curValue[0], curValue[1], curValue[2], curValue[3]);//*((s32 *)curValue);
				s32 R = BUILD_S32(reportableChange[0], reportableChange[1], reportableChange[2], reportableChange[3]);//*((s32 *)reportableChange);
				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
			/*
		case ZCL_DATA_TYPE_SINGLE_PREC:
			{
				float P = *((float *)prevValue);
				float C = *((float *)curValue);
				float R = *((float *)reportableChange);
				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
		case ZCL_DATA_TYPE_DOUBLE_PREC:
			{
				double P = *((double *)prevValue);
				double C = *((double *)curValue);
				double R = *((double *)reportableChange);
				if(P > C){
					needReport = ((P - C) >= R) ? TRUE : FALSE;
				}else if(P < C){
					needReport = ((C - P) >= R) ? TRUE : FALSE;
				}
			}
			break;
			*/
		default:
			break;
	}

	return needReport;
}

/*********************************************************************
 * @fn      reportAttrs
 *
 * @brief
 *
 * @param
 *
 * @return	NULL
 */
_CODE_ZCL_ void reportAttrs(void) {
    struct report_t {
        u8 numAttr;
        zclReport_t attr[2];
    };

    struct report_t report;

    bool again = 0;
    u16 profileID = 0xFFFF;
    u16 clusterID = 0xFFFF;
    u8 endpoint = 0;
    reportCfgInfo_t *pEntry = NULL;
    zclAttrInfo_t *pAttrEntry = NULL;

    do {
        pEntry = NULL;
        pAttrEntry = NULL;

        clusterID = 0xFFFF;
        endpoint = 0;
        again = 0;
        memset((u8*) &report, 0, sizeof(report));

        for (u8 i = 0; i < ZCL_REPORTING_TABLE_NUM; i++) {
            pEntry = &reportingTab.reportCfgInfo[i];

            if (pEntry->used && (pEntry->maxInterval != 0xFFFF) && zb_bindingTblSearched(pEntry->clusterID, pEntry->endPoint)) {
                pAttrEntry = zcl_findAttribute(pEntry->endPoint, pEntry->clusterID, pEntry->attrID);
                if (pAttrEntry) {
                    bool valid = 0;
                    u8 dataLen = zcl_getAttrSize(pAttrEntry->type, pAttrEntry->data);

                    if (!pEntry->maxIntCnt) {
                        if (!pEntry->maxInterval) {
                            if ((!zcl_analogDataType(pAttrEntry->type) && memcmp(pEntry->prevData, pAttrEntry->data, dataLen)) ||
                                 (zcl_analogDataType(pAttrEntry->type) && reportableChangeValueChk(pAttrEntry->type,
                                                                                                   pAttrEntry->data,
                                                                                                   pEntry->prevData,
                                                                                                   pEntry->reportableChange))) {
                                valid = 1;
                            }
                        } else {
                            valid = 1;
                        }
                    } else if (!pEntry->minIntCnt) {
                        if ((!zcl_analogDataType(pAttrEntry->type) && memcmp(pEntry->prevData, pAttrEntry->data, dataLen)) ||
                             (zcl_analogDataType(pAttrEntry->type) && reportableChangeValueChk(pAttrEntry->type,
                                                                                               pAttrEntry->data,
                                                                                               pEntry->prevData,
                                                                                               pEntry->reportableChange))) {
                            valid = 1;
                        } else {
                            pEntry->minIntCnt = pEntry->minInterval;
                        }
                    }

                    if (valid) {
                        if (clusterID == 0xFFFF) {
                            clusterID = pEntry->clusterID;
                            profileID = pEntry->profileID;
                            endpoint = pEntry->endPoint;
                        } else if ((clusterID != pEntry->clusterID)
                                || (profileID != pEntry->profileID)
                                || (endpoint != pEntry->endPoint)) {
                            again = 1;
                            continue;
                        }

                        report.attr[report.numAttr].attrID = pAttrEntry->id;
                        report.attr[report.numAttr].dataType = pAttrEntry->type;
                        report.attr[report.numAttr].attrData = pAttrEntry->data;
                        report.numAttr++;

                        //store for next compare
                        memcpy(pEntry->prevData, pAttrEntry->data, dataLen);
                        pEntry->minIntCnt = pEntry->minInterval;
                        pEntry->maxIntCnt = pEntry->maxInterval;

                        if (report.numAttr >= 2) {
                            if (!pEntry->minInterval) {
                                break;
                            }
                            again = 1;
//                            printf("again\r\n");
                            break;
                        }
                    }
                }
            }
        }

        if (clusterID != 0xFFFF) {
            epInfo_t dstEpInfo;
            TL_SETSTRUCTCONTENT(dstEpInfo, 0);

            dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;
            dstEpInfo.profileId = profileID;

//            printf("Test\r\n");
            report_divisor_multiplier(endpoint, clusterID, profileID, (zclReportCmd_t*)&report);

            zcl_sendReportAttrsCmd(endpoint, &dstEpInfo, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR, clusterID, (zclReportCmd_t* )&report);
        }
    } while (again);
}

/*********************************************************************
 * @fn      reportAttrTimerCb
 *
 * @brief
 *
 * @param   arg
 *
 * @return	0 -- continue; -1 -- cancel
 */
_CODE_ZCL_ static s32 reportAttrTimerCb(void *arg)
{
	u16 seconds = (u16)((u32)arg);

	if(zcl_reportingEntryActiveNumGet()){
		for(u8 i = 0; i < ZCL_REPORTING_TABLE_NUM; i++){
			reportCfgInfo_t *pEntry = &reportingTab.reportCfgInfo[i];

			if(pEntry->used && (pEntry->maxInterval != 0xFFFF) &&
			   zb_bindingTblSearched(pEntry->clusterID, pEntry->endPoint)){
				if(pEntry->minIntCnt){
					if(pEntry->minIntCnt >= seconds){
						pEntry->minIntCnt -= seconds;
					}else{
						pEntry->minIntCnt = 0;
					}
				}
				if(pEntry->maxIntCnt){
					if(pEntry->maxIntCnt >= seconds){
						pEntry->maxIntCnt -= seconds;
					}else{
						pEntry->maxIntCnt = 0;
					}
				}
			}
		}
	}

	reportAttrTimerEvt = NULL;
	return -1;
}

/*********************************************************************
 * @fn      reportAttrTimerStart
 *
 * @brief
 *
 * @param   second
 *
 * @return	NULL
 */
_CODE_ZCL_ void reportAttrTimerStart(void)
{
	u16 seconds = 0xFFFF;

	if(reportAttrTimerEvt){
		return;
	}

	for(u8 i = 0; i < ZCL_REPORTING_TABLE_NUM; i++){
		reportCfgInfo_t *pEntry = &reportingTab.reportCfgInfo[i];

		if(pEntry->used && (pEntry->maxInterval != 0xFFFF) && zb_bindingTblSearched(pEntry->clusterID, pEntry->endPoint)){
			if(pEntry->maxIntCnt && (pEntry->maxIntCnt < seconds)){
				seconds = pEntry->maxIntCnt;
			}
			if(pEntry->minIntCnt && (pEntry->minIntCnt < seconds)){
				seconds = pEntry->minIntCnt;
			}
		}
	}

	if(seconds != 0xFFFF){
		reportAttrTimerEvt = TL_ZB_TIMER_SCHEDULE(reportAttrTimerCb, (void *)((u32)seconds), seconds * 1000);
	}
}

/*********************************************************************
 * @fn      reportAttrTimerStop
 *
 * @brief
 *
 * @param   NULL
 *
 * @return	NULL
 */
_CODE_ZCL_ void reportAttrTimerStop(void)
{
	if(reportAttrTimerEvt){
		TL_ZB_TIMER_CANCEL(&reportAttrTimerEvt);
	}
}

/*********************************************************************
 * @fn      report_handler
 *
 * @brief
 *
 * @param   NULL
 *
 * @return	NULL
 */
_CODE_ZCL_ void report_handler(void)
{
	if(zb_isDeviceJoinedNwk()){
		reportAttrs();
		reportAttrTimerStart();
	}
}
