#include "app_main.h"

struct report_t {
    u8 numAttr;
    zclReport_t attr[11];
};

static struct report_t report;

static void app_forceReportingMetering2(void *args) {

    epInfo_t dstEpInfo;
    TL_SETSTRUCTCONTENT(dstEpInfo, 0);

    dstEpInfo.profileId = HA_PROFILE_ID;
    dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;

    report.numAttr = 0;

    zclAttrInfo_t *pAttrEntry;

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CUSTOM_DEVICE_MODEL);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_METER_SERIAL_NUMBER);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CUSTOM_DATE_RELEASE);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    if (report.numAttr) {
        zcl_sendReportAttrsCmd(APP_ENDPOINT_1,
                               &dstEpInfo,
                               TRUE,
                               ZCL_FRAME_SERVER_CLIENT_DIR,
                               ZCL_CLUSTER_SE_METERING,
                               (zclReportCmd_t* )&report);
    }
}

static void app_forceReportingMetering(void *args) {

    epInfo_t dstEpInfo;
    TL_SETSTRUCTCONTENT(dstEpInfo, 0);

    dstEpInfo.profileId = HA_PROFILE_ID;
    dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;

    report.numAttr = 0;

    zclAttrInfo_t *pAttrEntry;


    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_MULTIPLIER);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_DIVISOR);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_SUMMATION_DELIVERD);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_TIER_1_SUMMATION_DELIVERD);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_TIER_2_SUMMATION_DELIVERD);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_TIER_3_SUMMATION_DELIVERD);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_TIER_4_SUMMATION_DELIVERD);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_REMAINING_BATTERY_LIFE);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    if (report.numAttr) {
        zcl_sendReportAttrsCmd(APP_ENDPOINT_1,
                               &dstEpInfo,
                               TRUE,
                               ZCL_FRAME_SERVER_CLIENT_DIR,
                               ZCL_CLUSTER_SE_METERING,
                               (zclReportCmd_t* )&report);
    }
}

static void app_forceReportingElectrical(void *args) {

    epInfo_t dstEpInfo;
    TL_SETSTRUCTCONTENT(dstEpInfo, 0);

    dstEpInfo.profileId = HA_PROFILE_ID;
    dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;

    report.numAttr = 0;

    zclAttrInfo_t *pAttrEntry;

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_VOLTAGE_MULTIPLIER);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_VOLTAGE_DIVISOR);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_RMS_VOLTAGE);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_POWER_MULTIPLIER);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_POWER_DIVISOR);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_ACTIVE_POWER);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_CURRENT_MULTIPLIER);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_CURRENT_DIVISOR);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_RMS_CURRENT);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    if (report.numAttr) {
        zcl_sendReportAttrsCmd(APP_ENDPOINT_1,
                               &dstEpInfo,
                               TRUE,
                               ZCL_FRAME_SERVER_CLIENT_DIR,
                               ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT,
                               (zclReportCmd_t* )&report);
    }
}

static void app_forceReportingDevTemperature(void *args) {

    epInfo_t dstEpInfo;
    TL_SETSTRUCTCONTENT(dstEpInfo, 0);

    dstEpInfo.profileId = HA_PROFILE_ID;
    dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;

    report.numAttr = 0;

    zclAttrInfo_t *pAttrEntry;

    pAttrEntry = zcl_findAttribute(APP_ENDPOINT_1, ZCL_CLUSTER_GEN_DEVICE_TEMP_CONFIG, ZCL_ATTRID_DEV_TEMP_CURR_TEMP);
    if (pAttrEntry) {
        report.attr[report.numAttr].attrID = pAttrEntry->id;
        report.attr[report.numAttr].dataType = pAttrEntry->type;
        report.attr[report.numAttr].attrData = pAttrEntry->data;
        report.numAttr++;
    }

    if (report.numAttr) {
        zcl_sendReportAttrsCmd(APP_ENDPOINT_1,
                               &dstEpInfo,
                               TRUE,
                               ZCL_FRAME_SERVER_CLIENT_DIR,
                               ZCL_CLUSTER_GEN_DEVICE_TEMP_CONFIG,
                               (zclReportCmd_t* )&report);
    }
}

static void app_all_forceReporting() {

    if (zb_isDeviceJoinedNwk()) {
        TL_SCHEDULE_TASK(app_forceReportingMetering, NULL);
        TL_SCHEDULE_TASK(app_forceReportingMetering2, NULL);
        TL_SCHEDULE_TASK(app_forceReportingElectrical, NULL);
        TL_SCHEDULE_TASK(app_forceReportingDevTemperature, NULL);
    }
}

void app_forcedReport(uint8_t endpoint, uint16_t claster_id, uint16_t attr_id) {

    if (zb_isDeviceJoinedNwk()) {

        epInfo_t dstEpInfo;
        TL_SETSTRUCTCONTENT(dstEpInfo, 0);

        dstEpInfo.profileId = HA_PROFILE_ID;
#if FIND_AND_BIND_SUPPORT
        dstEpInfo.dstAddrMode = APS_DSTADDR_EP_NOTPRESETNT;
#else
        dstEpInfo.dstAddrMode = APS_SHORT_DSTADDR_WITHEP;
        dstEpInfo.dstEp = endpoint;
        dstEpInfo.dstAddr.shortAddr = 0xfffc;
#endif
        zclAttrInfo_t *pAttrEntry = zcl_findAttribute(endpoint, claster_id, attr_id);

        if (!pAttrEntry) {
            //should not happen.
            ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_ZCL_ENTRY);
            return;
        }

        zcl_sendReportCmd(endpoint, &dstEpInfo,  TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                claster_id, pAttrEntry->id, pAttrEntry->type, pAttrEntry->data);

#if UART_PRINTF_MODE && DEBUG_REPORTING
        printf("forceReportCb. endpoint: 0x%x, claster_id: 0x%x, attr_id: 0x%x\r\n", endpoint, claster_id, attr_id);
#endif
    }


}

int32_t forcedReportCb(void *arg) {

    app_all_forceReporting();

    return -1;
}
