/********************************************************************************************************
 * @file    zcl_illuminance_measurement.h
 *
 * @brief   This is the header file for zcl_illuminance_measurement
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
#ifndef ZCL_ILLUMINANCE_MEASUREMENT_H
#define ZCL_ILLUMINANCE_MEASUREMENT_H


/*********************************************************************
 * CONSTANTS
 */
/* Attribute ID */
#define ZCL_ATTRID_MEASURED_VALUE               0x0000
#define ZCL_ATTRID_MIN_MEASURED_VALUE           0x0001
#define ZCL_ATTRID_MAX_MEASURED_VALUE           0x0002
#define ZCL_ATTRID_TOLERANCE                    0x0003
#define ZCL_ATTRID_LIGHT_SENSOR_TYPE            0x0004


status_t zcl_illuminanceMeasure_register(u8 endpoint, u16 manuCode, u8 attrNum, const zclAttrInfo_t attrTbl[], cluster_forAppCb_t cb);

#endif	/* ZCL_ILLUMINANCE_MEASUREMENT_H */
