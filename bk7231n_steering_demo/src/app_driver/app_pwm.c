/*
 * @file name: 
 * @Descripttion: 
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-03-03 17:17:36
 * @LastEditors: zgw
 * @LastEditTime: 2021-03-26 11:28:25
 */

#include "app_pwm.h"
#include "BkDriverPwm.h"
#include "tuya_gpio.h"
#include "BkDriverGpio.h"

/***********************************************************
*************************pwm define*************************
***********************************************************/

#define PWM_CHANNEL_PORT_1                (6)
#define PWM_CHANNEL_PORT_2                (7)
#define PWM_CHANNEL_PORT_3                (8)

#define PWM_FREQ                          500


STATIC USER_PWM_INIT_T pwm_init_config ;          ///< PWM Settings
STATIC UCHAR_T user_pwm_init_flag = FALSE;
STATIC UCHAR_T *pChannelList = NULL;


STATIC USHORT_T pwm_period = 0;

STATIC CONST UCHAR_T gpio_allow_list[6][2] = {
                                                {6,  0},
                                                {7,  1},
                                                {8,  2},
                                                {9,  3},
                                                {24, 4},
                                                {26, 5}
                                            };

OPERATE_RET app_pwm_init(VOID)
{  
    
    OPERATE_RET opRet = -1;
    UCHAR_T i = 0, j = 0;
    UINT_T duty = 0;
    
    pwm_init_config.usFreq = PWM_FREQ;
    pwm_init_config.usDuty = 0;
    pwm_init_config.ucList[0] = PWM_CHANNEL_PORT_1;
    pwm_init_config.ucList[1] = PWM_CHANNEL_PORT_2;
    pwm_init_config.ucList[2] = PWM_CHANNEL_PORT_3;
    pwm_init_config.ucChannelNum = 3;
    pwm_init_config.bPolarity = 0;

    pChannelList = (UCHAR_T *)Malloc( pwm_init_config.ucChannelNum * SIZEOF(UCHAR_T) );

    for(i = 0; i < pwm_init_config.ucChannelNum; i++) {
        for(j = 0; j < 6; j++) {
            if(pwm_init_config.ucList[i] == gpio_allow_list[j][0]) {
                pChannelList[i] = gpio_allow_list[j][1];
                break;
            }
        }
        if( j >= 6 ) {
            break;
        }
    }
    
    pwm_period = (ULONG_T) (26000000 / pwm_init_config.usFreq);
    PR_NOTICE("pwm_period = %d",pwm_period);
    duty = (UINT_T) (pwm_init_config.usDuty / 1000.0 * pwm_period);

    for(i = 0; i < pwm_init_config.ucChannelNum; i++ ) {
        bk_pwm_initialize(pChannelList[i], pwm_period, duty, 0, 0);
        bk_pwm_stop(pChannelList[i]);
    }
    
    for(i = 0; i < pwm_init_config.ucChannelNum; i++ ) {
        bk_pwm_start(pChannelList[i]);
    }

    user_pwm_init_flag = TRUE;

    return OPRT_OK;
}

OPERATE_RET app_pwm_set(PWM_CHANNEL channel, UINT_T duty)
{
    FLOAT_T percent = 0.0;
    UCHAR_T i;
    UINT_T Data;
    if(user_pwm_init_flag != TRUE){
        PR_ERR("user pwm not init!");
        return OPRT_INVALID_PARM;
    }

    percent = (FLOAT_T)(duty/1000.0);
    
    bk_pwm_stop(pChannelList[channel]);
    Data = (UINT32)(percent * pwm_period);
    PR_NOTICE("----------------DUTY = %d----------",Data);
    bk_pwm_update_param(pChannelList[channel], pwm_period, (UINT32)(percent * pwm_period), 0, 0);

    bk_pwm_start(pChannelList[channel]);

    return OPRT_OK;
}

OPERATE_RET app_pwm_off(void)
{
    UCHAR_T i = 0;
    
    for(i = 0; i < pwm_init_config.ucChannelNum; i++) {
        bk_pwm_stop(pChannelList[i]);
    }
    
    return OPRT_OK;
}