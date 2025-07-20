#include "app_main.h"

int32_t flashLedStatusCb(void *arg) {

    if (!button_idle()) {
        if (zb_isDeviceJoinedNwk() && device_online) {
            light_blink_stop();
            if (fault_measure_flag || dev_config.device_model == DEVICE_UNDEFINED) {
                light_blink_start(2, 30, 200);
            } else {
                light_blink_start(1, 30, 30);
            }
        } else {
            if (!g_appCtx.net_steer_start) {
                if (fault_measure_flag || dev_config.device_model == DEVICE_UNDEFINED) {
                    light_blink_start(4, 30, 200);
                } else {
                    light_blink_start(3, 30, 200);
                }
            }
        }
    }

    return 0;
}

void led_on(uint32_t pin){
    drv_gpio_write(pin, LED_ON);
}

void led_off(uint32_t pin){
    drv_gpio_write(pin, LED_OFF);
}

void led_init(void){
    led_off(LED_POWER);
    led_off(LED_STATUS);
}

void light_on(void)
{
    led_on(LED_STATUS);
}

void light_off(void)
{
    led_off(LED_STATUS);
}

void light_init(void)
{
    led_init();
    led_on(LED_POWER);

}

int32_t zclLightTimerCb(void *arg)
{
    uint32_t interval = 0;

    if(g_appCtx.sta == g_appCtx.oriSta){
        g_appCtx.times--;
        if(g_appCtx.times <= 0){
            g_appCtx.timerLedEvt = NULL;
            return -1;
        }
    }

    g_appCtx.sta = !g_appCtx.sta;
    if(g_appCtx.sta){
        light_on();
        interval = g_appCtx.ledOnTime;
    }else{
        light_off();
        interval = g_appCtx.ledOffTime;
    }

    return interval;
}

void light_blink_start(uint8_t times, uint16_t ledOnTime, uint16_t ledOffTime)
{
    uint32_t interval = 0;
    g_appCtx.times = times;

    if(!g_appCtx.timerLedEvt){
        if(g_appCtx.oriSta){
            light_off();
            g_appCtx.sta = 0;
            interval = ledOffTime;
        }else{
            light_on();
            g_appCtx.sta = 1;
            interval = ledOnTime;
        }
        g_appCtx.ledOnTime = ledOnTime;
        g_appCtx.ledOffTime = ledOffTime;

        g_appCtx.timerLedEvt = TL_ZB_TIMER_SCHEDULE(zclLightTimerCb, NULL, interval);
    }
}

void light_blink_stop(void)
{
    if(g_appCtx.timerLedEvt){
        TL_ZB_TIMER_CANCEL(&g_appCtx.timerLedEvt);

        g_appCtx.times = 0;
        if(g_appCtx.oriSta){
            light_on();
        }else{
            light_off();
        }
    }
}

