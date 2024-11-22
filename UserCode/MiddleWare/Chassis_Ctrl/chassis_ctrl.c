/**
 * @file classis_ctrl.c
 * @author Lary
 * @brief 三全向轮底盘
 * @version 0.1
 * @date 2024-11-04
 * 
 * 方向定义：
 *      底盘方向定义
 *         ^x(wheel0)
 *         |
 *     y   |
 *    <————.   轮组0的轴线和x正方向重合，轮组1，2的中心和底盘中心的连线与y轴夹角为30度
 *        
 * @copyright Copyright (c) 2024
 * 
 */

#include "chassis_ctrl.h"


/**********************************************************
 * @brief 线程定义
 * 
 */
osThreadId_t can_message_TaskHandle;
const osThreadAttr_t can_message_Task_attributes = {
    .name       = "can_message_Task",
    .stack_size = 256 * 4,
    .priority   = (osPriority_t)osPriorityHigh1,
};
void my_CAN_Message_Task(void *argument);

osThreadId_t chassis_ctrl_TaskHandle;
const osThreadAttr_t chassis_ctrl_Task_attributes = {
    .name       = "chassis_ctrl_Task",
    .stack_size = 256 * 4,
    .priority   = (osPriority_t)osPriorityHigh1,
};
void my_Chassis_Ctrl_Task(void *argument);

/*********************************************************
 * @brief 数据接口定义
 * 
 */

//float wheel0_velocity, wheel1_velocity, wheel2_velocity;            //底盘目标电机速度

Alldir_Chassis_t my_Alldir_Chassis_t;

/***********************************************************
 * @brief 底盘整体运动控制
 * 
 */


/************************************************************
 * @brief 底盘初始化
 * 
 */
void my_Chassis_Init(void)
{
    CANFilterInit(&hcan1);
    CAN2FilterInit(&hcan2);
    for (int i = 0; i < 4; i++){
        hDJI[i].motorType = M3508;
    }
    DJI_Init();
    for (int i = 0; i < 4; i++) {
        hDJI[i].posPID.KI=1.5;
        hDJI[i].posPID.KD=0;
        hDJI[i].speedPID.outputMax=8000;
        hDJI[i].speedPID.KI=0.02;
        hDJI[i].speedPID.KD=0.1;
        hDJI[i].speedPID.KP=0.8;
    }    
}

/***************************************************************
 * @brief   底盘电机CAN消息发送线程创建
 */
void my_Chassis_CAN_Message_TaskStart(void)
{
    can_message_TaskHandle = osThreadNew(my_CAN_Message_Task, NULL, &can_message_Task_attributes);
}

/**************************************************************
 * @brief   底盘控制启动线程
 */
void my_Chassis_Ctrl_TaskStart(void)
{
    chassis_ctrl_TaskHandle = osThreadNew(my_Chassis_Ctrl_Task, NULL, &chassis_ctrl_Task_attributes);
}

/**************************************************************
 * @brief 线程函数，底盘电机CAN发送消息线程
 * 
 * @param arguement 
 */
void my_CAN_Message_Task(void *arguement)
{
    for(;;)
    {
        speedServo(my_Alldir_Chassis_t.my_wheel[0].target_v,&hDJI[0]);
        speedServo(my_Alldir_Chassis_t.my_wheel[1].target_v,&hDJI[1]);
        speedServo(my_Alldir_Chassis_t.my_wheel[2].target_v,&hDJI[2]);
        CanTransmit_DJI_1234(&hcan1,hDJI[0].speedPID.output,hDJI[1].speedPID.output,hDJI[2].speedPID.output,hDJI[3].speedPID.output);
        osDelay(1);
    }
}

/**************************************************************
 * @brief 线程函数，底盘电机控制线程
 * 
 * @param arguement 
 */
void my_Chassis_Ctrl_Task(void *arguement)
{
    for(;;)
    {
        if(my_Alldir_Chassis_t.state == CHASSIS_HANDLE_RUNNING)
        {
            my_Alldir_Chassis_t.target_v.vx = MyRemote_Data.usr_left_y;
            my_Alldir_Chassis_t.target_v.vy = MyRemote_Data.usr_left_x;
            my_Alldir_Chassis_t.target_v.vw = MyRemote_Data.usr_right_x;
        }
        Inverse_kinematic_equation(my_Alldir_Chassis_t);
    }
}

/**************************************************************
 * @brief 三全向轮底盘逆运动学解算
 * 
 * @param vx        底盘前向速度，单位m/s
 * @param vy        底盘左横向速度，单位m/s
 * @param wc        转向角速度，使用弧度，单位rad/s
 * @param out_v0    
 * @param out_v1    三个输出电机速度,单位rpm
 * @param out_v2 
 */
void Inverse_kinematic_equation(Alldir_Chassis_t this_chassis)
{
    float v0,v1,v2;
    v0 = (float)((this_chassis.target_v.vy + this_chassis.target_v.vw * chassis_r) / chassis_R);
    v1 = (float)((-sqrt(3) * this_chassis.target_v.vx / 2 - this_chassis.target_v.vy / 2 + this_chassis.target_v.vw * chassis_r) / chassis_R);
    v2 = (float)((sqrt(3) * this_chassis.target_v.vx / 2 - this_chassis.target_v.vy / 2 + this_chassis.target_v.vw * chassis_r) / chassis_R);

    this_chassis.my_wheel[0].target_v = mps_to_rpm(v0);
    this_chassis.my_wheel[1].target_v = mps_to_rpm(v1);
    this_chassis.my_wheel[2].target_v = mps_to_rpm(v2);
}

