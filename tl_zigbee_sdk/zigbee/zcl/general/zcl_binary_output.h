/********************************************************************************************************
 * @file    zcl_binary_output.h
 *
 * @brief   This is the header file for zcl_binary_output
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
#ifndef ZCL_BINARY_OUTPUT_H
#define ZCL_BINARY_OUTPUT_H


/*********************************************************************
 * CONSTANTS
 */

/**
 *  @brief	binary cluster Attribute IDs
 */
#define ZCL_BINARY_OUTPUT_ATTRID_ACTIVE_TEXT            0x0004
#define ZCL_BINARY_OUTPUT_ATTRID_DESCRIPTION            0x001C
#define ZCL_BINARY_OUTPUT_ATTRID_INACTIVE_TEXT          0x002E
#define ZCL_BINARY_OUTPUT_ATTRID_MIN_OFF_TIME           0x0042
#define ZCL_BINARY_OUTPUT_ATTRID_MIN_ON_TIME            0x0043
#define ZCL_BINARY_OUTPUT_ATTRID_OUT_OF_SERVICE         0x0051
#define ZCL_BINARY_OUTPUT_ATTRID_POLARITY               0x0054
#define ZCL_BINARY_OUTPUT_ATTRID_PRESENT_VALUE          0x0055
#define ZCL_BINARY_OUTPUT_ATTRID_PRIORITY_ARRAY         0x0057
#define ZCL_BINARY_OUTPUT_ATTRID_RELIABLILITY           0x0067
#define ZCL_BINARY_OUTPUT_ATTRID_RELINQUISH_DEFAULT     0x0068
#define ZCL_BINARY_OUTPUT_ATTRID_STATUS_FLAGS           0x006F
#define ZCL_BINARY_OUTPUT_ATTRID_APP_TYPE               0x0100

/**
 *  @brief Max length of char string
 */
#define ZCL_BINARY_OUTPUT_MAX_LENGTH                    24


/**
 *  @brief  Definition format for priority array
 */
typedef struct {
    bool priority1;
    bool priority2;
} zcl_binOutput_priorityArray_t;

#if 0
#ifdef ZCL_BINARY_OUTPUT_ATTR_ACTIVE_TEXT_ENABLE
extern u8 zcl_attr_binOutput_activeText[];
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_DESCRIPTIONT_ENABLE
extern u8 zcl_attr_binOutput_description[];
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_INACTIVE_TEXT_ENABLE
extern u8 zcl_attr_binOutput_inactiveText[];
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_MIN_OFF_TIME_ENABLE
extern u32 zcl_attr_binOutput_minOffTime;
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_MIN_ON_TIME_ENABLE
extern u32 zcl_attr_binOutput_minOnTime;
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_OUT_OF_SERVICE_ENABLE
extern bool zcl_attrr_binOutput_outOfService;
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_POLARITY_ENABLE
extern u8 zcl_attr_binOutput_polarity;
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_PRESENT_VALUE_ENABLE
extern bool zcl_attr_binOutput_presentValue;
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_PRIORITY_ARRAY_ENABLE
extern zcl_binOutput_priorityArray_t zcl_attr_binOutput_priorityArray[];
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_RELIABILITY_ENABLE
extern u8 zcl_attr_binOutput_reliability;
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_RELINQUISH_DEFAULT_ENABLE
extern bool zcl_attr_binOutput_relinquishDefault;
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_STATUS_FLAGS_ENABLE
extern u8 zcl_attr_binOutput_statusFlags;
#endif
#ifdef ZCL_BINARY_OUTPUT_ATTR_APP_TYPE_ENABLE
extern u32 zcl_attr_binOutput_appType;
#endif

extern const zclAttrInfo_t binary_output_attrTbl[];
extern const u8 zcl_binary_output_attrNum;
#endif


status_t zcl_binary_output_register(u8 endpoint, u16 manuCode, u8 attrNum, const zclAttrInfo_t attrTbl[], cluster_forAppCb_t cb);

#endif	/* ZCL_BINARY_OUTPUT_H */
