/**
 * @file Unitree.c
 * @author Lary (you@domain.com)
 * @brief  一代扣篮车底盘跳跃电机控制
 * @version 0.1
 * @date 2024-11-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "Unitree.h"
UnitreeMotor unitree_DunkMotor_t[3];   //底盘跳跃电机对象创建

float unitree_DunkMotor_Tff[3];         //前馈力矩
float unitree_DunkMotor_Wdes[3];        //电机输出轴速度（-127.014 ~ 127.014  单位 rad/s）
float unitree_DunkMotor_posdes[3];        //期望电机输出轴输出位置（-65051.18 ~ 65051.18  单位 rad）
float unitree_DunkMotor_kp[3];          //电机刚度系数/位置误差比例系数（0 ~ 25.599）
float unitree_DunkMotor_kw[3];          //电机阻尼系数/速度误差比例（0 ~ 25.599）

/************************************************************
 * 线程定义
 */
osThreadId_t unitree_uart_message_TaskHandle;
const osThreadAttr_t unitree_uart_message_Task_attributes = {
    .name       = "unitree_uart_message_Task",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};
void my_Unitree_UART_Message_Task(void *argument);

osThreadId_t unitree_dunk_ctrl_TaskHandle;
const osThreadAttr_t unitree_dunk_ctrl_Task_attributes = {
    .name       = "unitree_dunk_ctrl_Task",
    .stack_size = 128*4,
    .priority   = (osPriority_t)osPriorityNormal,
};
void my_unitree_dunk_ctrl_Task(void *argument);


/***********************************************************接口函数************************************************************/
/**
 * @brief   Unitree电机初始化
 */
void my_Unitree_Init(void)
{
    for (uint8_t i = 0; i <2; i++)
    {
        if(Unitree_init(&unitree_DunkMotor_t[i], &UART_UNITREE_HANDLER, i) == HAL_OK)
        {
            HAL_GPIO_TogglePin(LED6_GPIO_Port,LED6_Pin);
            osDelay(1);
        }
    }
    
    
}

/**
 * @brief   Unitree串口消息发送线程创建
 */
void my_Unitree_UART_Message_TaskStart(void)
{
    unitree_uart_message_TaskHandle = osThreadNew(my_Unitree_UART_Message_Task, NULL, &unitree_uart_message_Task_attributes);
}

/**
 * @brief Unitree底盘跳跃电机控制线程创建
 * 
 */
void my_unitree_dunk_ctrl_TaskStart(void)
{
    unitree_dunk_ctrl_TaskHandle = osThreadNew(my_unitree_dunk_ctrl_Task,NULL,&unitree_dunk_ctrl_Task_attributes);
}


/**************************************************内部控制函数***********************************************************************/
/**
 * @brief 宇树电机零力矩模式，阻尼最小
 * 
 */
void Unitree_motor_0Torque(void)
{
    for (uint8_t i = 0; i < 15; i++)
    {
        Unitree_UART_tranANDrev(&unitree_DunkMotor_t[0],i,1,0,0,0,0,0);
        Unitree_UART_tranANDrev(&unitree_DunkMotor_t[1],i,1,0,0,0,0,0);
        Unitree_UART_tranANDrev(&unitree_DunkMotor_t[2],i,1,0,0,0,0,0);
    }
    
}

/**
 * @brief PD控制器力位混合控制，加入前馈力矩
 * 
 */
void Unitree_motor_PosAndTor(void)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        unitree_DunkMotor_t[i].cmd.T = unitree_DunkMotor_Tff[i] + unitree_DunkMotor_t[i].cmd.K_P * (unitree_DunkMotor_t[i].cmd.Pos - 
        unitree_DunkMotor_t[i].data.Pos) + unitree_DunkMotor_t[i].cmd.K_W * (unitree_DunkMotor_t[i].cmd.W - unitree_DunkMotor_t[i].data.W);
    }
     
}


/************************************************************线程函数**************************************************************************/
/**
 * @brief   底盘Unitree电机UART消息发送线程
 */
void my_Unitree_UART_Message_Task(void *argument)
{
    for (;;) {
        Unitree_UART_tranANDrev(&unitree_DunkMotor_t[0], 1, 1, unitree_DunkMotor_t[0].cmd.T, 5, unitree_DunkMotor_t[0].cmd.Pos, unitree_DunkMotor_t[0].cmd.K_P, unitree_DunkMotor_t[0].cmd.K_W);
        osDelay(2);
        /* Unitree_UART_tranANDrev(&unitree_DunkMotor_t[1], 0, 1, 0, 0, unitree_DunkMotor_posdes[1], unitree_DunkMotor_kp[1], unitree_DunkMotor_kw[1]);
        osDelay(2);
        Unitree_UART_tranANDrev(&unitree_DunkMotor_t[2], 0, 1, 0, 0, unitree_DunkMotor_posdes[2], unitree_DunkMotor_kp[2], unitree_DunkMotor_kw[2]);
        osDelay(2); */
        
    }
}

/** 
 * @brief 底盘Unitree跳跃电机控制线程
 * 
 */
void my_unitree_dunk_ctrl_Task(void *argument)
{
    for (; ;)
    {
        unitree_DunkMotor_posdes[0]=0;        //电机输出轴输出位置（-65051.18 ~ 65051.18  单位 rad）
        unitree_DunkMotor_kp[0] = 0;          //电机刚度系数/位置误差比例系数（0 ~ 25.599）
        unitree_DunkMotor_kw[0] = 0.02;
        osDelay(1);
    }
    
}
