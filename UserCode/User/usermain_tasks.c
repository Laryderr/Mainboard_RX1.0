/**
 * @file usermain_tasks.c
 * @author Lary (you@domain.com)
 * @brief WTR2025 RX 扣篮一代车版本工程
 * @note  整车总状态机，总运行线程
 * @version 0.1
 * @date 2024-11-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "usermain_tasks.h"

enum Running_Status running_status;

/******************************************************
 * @brief 定义总线程
 * 
 */
osThreadId_t main_TaskHandle;
const osThreadAttr_t main_Task_attributes = {
    .name       = "main_Task",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};
void my_main_Task(void *argument);



/**
 * @brief 总线程初始化（接口函数）
 * 
 */
void my_main_Task_Start(void)
{
    running_status       = HANDLE_MODE;
    main_TaskHandle = osThreadNew(my_main_Task, NULL, &main_Task_attributes);
}

/**
 * @brief 辅助函数
 * 
 */
int motor_wait_to_finish(float feedback, float setpoint)
{
    if (((feedback - setpoint) < -5.0f) || ((feedback - setpoint) > 5.0f)) {
        return 0;
    } else {
        return 1;
    }
}


/*****************************************************
 * @brief 线程函数定义
 * 
 */

/**
 * @brief 总线程实现
 * 
 */
void my_main_Task(void *arguement)
{
    my_handle_Task_Start();
    for(;;)
    {
        switch (my_Alldir_Chassis_t.state)
        {
        case CHASSIS_READY:
            if (MyRemote_Data.left_switch == 0)
            {
                my_Alldir_Chassis_t.state = CHASSIS_HANDLE_RUNNING;
            }
            break;
        case CHASSIS_HANDLE_RUNNING:
            //osThreadSuspend(auto_TaskHandle);
            //osThreadResume(handle_TaskHandle);
            //JoystickSwitchTitle(ID_RUN, run_title, &mav_run_title);
            //JoystickSwitchMsg(ID_RUN, run_handle_msg, &mav_run_msg);
            break;
        case CHASSIS_AUTO_RUNNING :
            break;
        case CHASSIS_STOP :
            my_Alldir_Chassis_t.target_v.vx = 0;
            my_Alldir_Chassis_t.target_v.vy = 0;
            my_Alldir_Chassis_t.target_v.vw = 0;
            break;
        case CHASSIS_ERROR:
            my_Alldir_Chassis_t.target_v.vx = 0;
            my_Alldir_Chassis_t.target_v.vy = 0;
            my_Alldir_Chassis_t.target_v.vw = 0;
            break;
        default:
            break;
        }
        osDelay(1);
    }
}