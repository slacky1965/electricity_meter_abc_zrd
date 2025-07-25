#include "app_main.h"

uint8_t device_online = false;
uint8_t resp_time = false;
static uint32_t last_report = 0;

app_ctx_t g_appCtx = {
        .timerFactoryReset = NULL,
        .net_steer_start = false,
        .oriSta = false,
};

#ifdef ZCL_OTA
extern ota_callBack_t app_otaCb;

//running code firmware information
ota_preamble_t app_otaInfo = {
	.fileVer 			= FILE_VERSION,
	.imageType 			= IMAGE_TYPE,
	.manufacturerCode 	= MANUFACTURER_CODE_TELINK,
};
#endif

//Must declare the application call back function which used by ZDO layer
const zdo_appIndCb_t appCbLst = {
    bdb_zdoStartDevCnf,//start device cnf cb
    NULL,//reset cnf cb
    NULL,//device announce indication cb
    app_leaveIndHandler,//leave ind cb
    app_leaveCnfHandler,//leave cnf cb
    app_nwkUpdateIndicateHandler,//nwk update ind cb
    NULL,//permit join ind cb
    NULL,//nlme sync cnf cb
    NULL,//tc join ind cb
    NULL,//tc detects that the frame counter is near limit
};


/**
 *  @brief Definition for bdb commissioning setting
 */
bdb_commissionSetting_t g_bdbCommissionSetting = {
    .linkKey.tcLinkKey.keyType = SS_GLOBAL_LINK_KEY,
    .linkKey.tcLinkKey.key = (uint8_t *)tcLinkKeyCentralDefault,             //can use unique link key stored in NV

    .linkKey.distributeLinkKey.keyType = MASTER_KEY,
    .linkKey.distributeLinkKey.key = (uint8_t *)linkKeyDistributedMaster,    //use linkKeyDistributedCertification before testing

    .linkKey.touchLinkKey.keyType = MASTER_KEY,
    .linkKey.touchLinkKey.key = (uint8_t *)touchLinkKeyMaster,               //use touchLinkKeyCertification before testing

#if TOUCHLINK_SUPPORT
    .touchlinkEnable = 1,                                               /* enable touch-link */
#else
    .touchlinkEnable = 0,                                               /* disable touch-link */
#endif
    .touchlinkChannel = DEFAULT_CHANNEL,                                /* touch-link default operation channel for target */
    .touchlinkLqiThreshold = 0xA0,                                      /* threshold for touch-link scan req/resp command */
};



/*********************************************************************
 * @fn      stack_init
 *
 * @brief   This function initialize the ZigBee stack and related profile. If HA/ZLL profile is
 *          enabled in this application, related cluster should be registered here.
 *
 * @param   None
 *
 * @return  None
 */
void stack_init(void)
{
	/* Initialize ZB stack */
	zb_init();

	/* Register stack CB */
    zb_zdoCbRegister((zdo_appIndCb_t *)&appCbLst);
}

/*********************************************************************
 * @fn      user_app_init
 *
 * @brief   This function initialize the application(Endpoint) information for this node.
 *
 * @param   None
 *
 * @return  None
 */
void user_app_init(void)
{
	af_nodeDescManuCodeUpdate(MANUFACTURER_CODE_TELINK);

    /* Initialize ZCL layer */
	/* Register Incoming ZCL Foundation command/response messages */
    zcl_init(app_zclProcessIncomingMsg);

	/* Register endPoint */
	af_endpointRegister(APP_ENDPOINT_1, (af_simple_descriptor_t *)&app_simpleDesc, zcl_rx_handler, NULL);

	zcl_reportingTabInit();

	/* Register ZCL specific cluster information */
	zcl_register(APP_ENDPOINT_1, APP_CB_CLUSTER_NUM, (zcl_specClusterInfo_t *)g_appClusterList);

#if ZCL_GP_SUPPORT
	/* Initialize GP */
	gp_init(APP_ENDPOINT_1);
#endif

#if ZCL_OTA_SUPPORT
	/* Initialize OTA */
    ota_init(OTA_TYPE_CLIENT, (af_simple_descriptor_t *)&app_simpleDesc, &app_otaInfo, &app_otaCb);
#endif

#if ZCL_WWAH_SUPPORT
    /* Initialize WWAH server */
    wwah_init(WWAH_TYPE_SERVER, (af_simple_descriptor_t *)&app_simpleDesc);
#endif

//    app_uart_init(); uart initialize from function set_device_model()
    init_config(true);

    ds18b20_init();

    /* start timer for flash status led */
    g_appCtx.timerLedStatusEvt = TL_ZB_TIMER_SCHEDULE(flashLedStatusCb, NULL, TIMEOUT_5SEC);

    /* start timer check net from coordinator - get time */
    TL_ZB_TIMER_SCHEDULE(getTimeCb, NULL, TIMEOUT_5MIN);

    /* start timer for control internal temperature */
    getTemperatureCb(NULL);
    TL_ZB_TIMER_SCHEDULE(getTemperatureCb, NULL, TIMEOUT_5SEC);

    /* set device model */
    set_device_model(dev_config.device_model);
}

void app_task(void) {

    button_handler();
    tamper_handler();

    if(BDB_STATE_GET() == BDB_STATE_IDLE){

        report_handler();
        if (clock_time_exceed(last_report, TIMEOUT_TICK_1SEC)) {
            last_report = clock_time();
        }

    }
}


extern volatile u16 T_evtExcept[4];

static void app_sysException(void) {

#if UART_PRINTF_MODE
    printf("app_sysException, line: %d, event: %d, reset\r\n", T_evtExcept[0], T_evtExcept[1]);
#endif

#if 1
    SYSTEM_RESET();
#else
    led_on(LED_STATUS);
    while(1);
#endif
}

/*********************************************************************
 * @fn      user_init
 *
 * @brief   User level initialization code.
 *
 * @param   isRetention - if it is waking up with ram retention.
 *
 * @return  None
 */
void user_init(bool isRetention)
{
    (void)isRetention;

    /* Initialize LEDs*/
    light_init();

    /* Initialize Stack */
    stack_init();

    /* Initialize user application */
    user_app_init();

    /* Register except handler for test */
    sys_exceptHandlerRegister(app_sysException);


    /* User's Task */
#if ZBHCI_EN
    zbhciInit();
    ev_on_poll(EV_POLL_HCI, zbhciTask);
#endif
    ev_on_poll(EV_POLL_IDLE, app_task);

    /* Read the pre-install code from NV */
    if(bdb_preInstallCodeLoad(&g_appCtx.tcLinkKey.keyType, g_appCtx.tcLinkKey.key) == RET_OK){
        g_bdbCommissionSetting.linkKey.tcLinkKey.keyType = g_appCtx.tcLinkKey.keyType;
        g_bdbCommissionSetting.linkKey.tcLinkKey.key = g_appCtx.tcLinkKey.key;
    }

//    /* Set default reporting configuration */
//    uint8_t reportableChange = 0x00;
//
//    /* Device temperature */
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_GEN_DEVICE_TEMP_CONFIG, ZCL_ATTRID_DEV_TEMP_CURR_TEMP,
//            0, 300, (uint8_t *)&reportableChange);
//
//    /* 4 tariffs and divisor, multiplier, battery life */
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_TIER_1_SUMMATION_DELIVERD,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_TIER_2_SUMMATION_DELIVERD,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_TIER_3_SUMMATION_DELIVERD,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_TIER_4_SUMMATION_DELIVERD,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CURRENT_SUMMATION_DELIVERD,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_STATUS,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_MULTIPLIER,
//            0, 0, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_DIVISOR,
//            0, 0, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_REMAINING_BATTERY_LIFE,
//            0, 300, (uint8_t *)&reportableChange);
//
//    /* Serial Number, Data Release and Model Name */
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_METER_SERIAL_NUMBER,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CUSTOM_DATE_RELEASE,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_SE_METERING, ZCL_ATTRID_CUSTOM_DEVICE_MODEL,
//            0, 300, (uint8_t *)&reportableChange);
//
//    /* Voltage and divisor, multiplier */
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_RMS_VOLTAGE,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_VOLTAGE_MULTIPLIER,
//            0, 0, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_VOLTAGE_DIVISOR,
//            0, 0, (uint8_t *)&reportableChange);
//
//    /* Current and divisor, multiplier */
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_RMS_CURRENT,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_CURRENT_MULTIPLIER,
//            0, 0, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_CURRENT_DIVISOR,
//            0, 0, (uint8_t *)&reportableChange);
//
//    /* Power and divisor, multiplier */
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_ACTIVE_POWER,
//            0, 300, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_POWER_MULTIPLIER,
//            0, 0, (uint8_t *)&reportableChange);
//    bdb_defaultReportingCfg(APP_ENDPOINT_1, HA_PROFILE_ID, ZCL_CLUSTER_MS_ELECTRICAL_MEASUREMENT, ZCL_ATTRID_AC_POWER_DIVISOR,
//            0, 0, (uint8_t *)&reportableChange);

    /* Initialize BDB */
    bdb_init((af_simple_descriptor_t *)&app_simpleDesc, &g_bdbCommissionSetting, &g_zbBdbCb, 1);
}

