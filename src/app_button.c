#include "tl_common.h"
#include "zcl_include.h"

#include "app_main.h"

bool factory_reset = false;

static int32_t net_steer_start_offCb(void *args) {

    g_appCtx.net_steer_start = false;
    factory_reset = false;

    return -1;
}

//static int32_t factory_resetCb(void *args) {
//
//    g_appCtx.net_steer_start = true;
//    TL_ZB_TIMER_SCHEDULE(net_steer_start_offCb, NULL, TIMEOUT_1MIN30SEC);
//    led_effect_start(55, COLOR_RED);
//
//    return -1;
//}


static void buttonKeepPressed(uint8_t btNum) {
    g_appCtx.button[btNum-1].state = APP_FACTORY_NEW_DOING;
    g_appCtx.button[btNum-1].ctn = 0;

    if(btNum == VK_SW1) {
#if UART_PRINTF_MODE && DEBUG_BUTTON
        printf("The button was keep pressed for 5 seconds\r\n");
#endif

        if (zb_getLocalShortAddr() >= 0xFFF8) {
//            printf("1 Factory new: %s\r\n", zb_isDeviceFactoryNew()?"yes":"no");
            if (!factory_reset) {
                factory_reset = true;
                zb_resetDevice2FN();
//                printf("1.2 Factory new: %s\r\n", zb_isDeviceFactoryNew()?"yes":"no");
            }
        } else {
            zb_resetDevice2FN();
            sleep_ms(200);
            if (g_appCtx.timerFactoryReset) {
                TL_ZB_TIMER_CANCEL(&g_appCtx.timerFactoryReset);
            }
            g_appCtx.timerFactoryReset = TL_ZB_TIMER_SCHEDULE(delayedFactoryResetCb, NULL, TIMEOUT_5SEC);
//            printf("2 Factory new: %s\r\n", zb_isDeviceFactoryNew()?"yes":"no");
        }

        g_appCtx.net_steer_start = true;
        TL_ZB_TIMER_SCHEDULE(net_steer_start_offCb, NULL, TIMEOUT_1MIN30SEC);
        light_blink_start(90, 100, 1000);
    }
}

static void buttonCheckCommand(uint8_t btNum) {
    g_appCtx.button[btNum-1].state = APP_STATE_NORMAL;

    if (g_appCtx.button[btNum-1].ctn == 1) {
#if UART_PRINTF_MODE && DEBUG_BUTTON
        printf("Button push 1 time\r\n");
#endif
        TL_ZB_TIMER_SCHEDULE(forcedReportCb, NULL, TIMEOUT_100MS);
     } else if (g_appCtx.button[btNum-1].ctn == 3) {
#if UART_PRINTF_MODE && DEBUG_BUTTON
        printf("Button push 3 time\r\n");
#endif
        TL_ZB_TIMER_SCHEDULE(delayedMcuResetCb, NULL, TIMEOUT_100MS);
     }

    g_appCtx.button[btNum-1].ctn = 0;
}


void keyScan_keyPressedCB(kb_data_t *kbEvt) {

    uint8_t keyCode = kbEvt->keycode[0];

    if(keyCode != 0xff) {
        g_appCtx.button[keyCode-1].pressed_time = clock_time();
        g_appCtx.button[keyCode-1].state = APP_FACTORY_NEW_SET_CHECK;
        g_appCtx.button[keyCode-1].ctn++;
        light_blink_start(1, 30, 1);
    }
}


void keyScan_keyReleasedCB(uint8_t keyCode) {

    if (keyCode != 0xff) {
        g_appCtx.button[keyCode-1].released_time = clock_time();
        g_appCtx.button[keyCode-1].state = APP_STATE_RELEASE;
    }
}

void button_handler(void) {
    static uint8_t valid_keyCode = 0xff;

    for (uint8_t i = 0; i < MAX_BUTTON_NUM; i++) {
        if (g_appCtx.button[i].state == APP_FACTORY_NEW_SET_CHECK) {
            if(clock_time_exceed(g_appCtx.button[i].pressed_time, TIMEOUT_TICK_5SEC)) {
                buttonKeepPressed(i+1);
            }
        }

        if (g_appCtx.button[i].state == APP_STATE_RELEASE) {
            if(clock_time_exceed(g_appCtx.button[i].released_time, TIMEOUT_TICK_250MS)) {
                buttonCheckCommand(i+1);
            }

        }
    }

    if(kb_scan_key(0, 1)){
        if(kb_event.cnt){
            g_appCtx.keyPressed = 1;
            keyScan_keyPressedCB(&kb_event);
            if(kb_event.cnt == 1){
                valid_keyCode = kb_event.keycode[0];
            }
        }else{
            keyScan_keyReleasedCB(valid_keyCode);
            valid_keyCode = 0xff;
            g_appCtx.keyPressed = 0;
        }
    }
}

uint8_t button_idle() {

    if (g_appCtx.keyPressed) {
        return true;
    }

    for (uint8_t i = 0; i < MAX_BUTTON_NUM; i++) {
        if (g_appCtx.button[i].ctn) return true;
    }

    return false;
}
