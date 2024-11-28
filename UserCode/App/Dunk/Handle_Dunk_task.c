/**
 * @file Handle_Dunk_task.c
 * @author Lary (you@domain.com)
 * @brief 扣篮实现线程
 * @version 0.1
 * @date 2024-11-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "Unitree.h"
#include "Handle_Dunk_task.h"


float Unitree_DunkMotor_LimitingPos[2][3]= {0,0,0,0,0,0}; // 0:抬腿极限位置，1：伸腿极限位置
bool Found_LimitingPos_Flag = 0;
enum Dunk_Status my_Dunk_Status;

/****************************************************线程定义******************************************/
osThreadId_t unitree_dunk_ctrl_TaskHandle;
const osThreadAttr_t unitree_dunk_ctrl_Task_attributes = {
    .name       = "unitree_dunk_ctrl_Task",
    .stack_size = 128*4,
    .priority   = (osPriority_t)osPriorityNormal,
};
void Handle_Dunk_Task(void *argument);



/**
 * @brief Unitree底盘跳跃电机控制线程创建
 * 
 */
void Handle_Dunk_TaskStart(void)
{
    unitree_dunk_ctrl_TaskHandle = osThreadNew(Handle_Dunk_Task,NULL,&unitree_dunk_ctrl_Task_attributes);
}



/**************************************************内部控制函数***********************************************************************/
/**
 * @brief 宇树电机零力矩模式，阻尼最小
 * 
 */
void Unitree_motor_0Torque(void)
{
    for (uint8_t i = 0; i <16; i++)
    {
        unitree_DunkMotor_t[i].cmd.T = 0;
        unitree_DunkMotor_t[i].cmd.K_P = 0;
        unitree_DunkMotor_t[i].cmd.W = 0;
        unitree_DunkMotor_t[i].cmd.K_W = 0.02;
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

/**
 * @brief 寻找跳跃电机姿态极限位置
 * 
 * @return float 
 */
float Dunkmotor_Contect_LimitingPos(uint8_t i)
{
        unitree_DunkMotor_t[i].cmd.K_P = 0.02;
        unitree_DunkMotor_t[i].cmd.Pos = 0;
        unitree_DunkMotor_t[i].cmd.W = 1;
        unitree_DunkMotor_t[i].cmd.T = 0.3;
        unitree_DunkMotor_t[i].cmd.K_W =0.05;
        //寻找抬腿极限
        if (i > 0)
        {
           return unitree_DunkMotor_t[i].data.Pos;
        }

        if (unitree_DunkMotor_t[i].data.T >= 0.4 )
        {
            unitree_DunkMotor_t[i].cmd.W = 0;
            unitree_DunkMotor_t[i].cmd.T = 0.15;
            return unitree_DunkMotor_t[i].data.Pos;
        }
    
}


/** 
 * @brief 底盘Unitree跳跃电机控制线程
 * 
 */
void Handle_Dunk_Task(void *argument)
{
    for (; ;)
    {
        //判断抬腿极限位置
        if (Found_LimitingPos_Flag == 0)
        {
                for (uint8_t i = 0; i < 10; i++)
                {
                    Unitree_DunkMotor_LimitingPos[0][0] = Unitree_DunkMotor_LimitingPos[0][0] + Dunkmotor_Contect_LimitingPos(0);
                    Unitree_DunkMotor_LimitingPos[0][1] = Unitree_DunkMotor_LimitingPos[0][1] + Dunkmotor_Contect_LimitingPos(1);
                    Unitree_DunkMotor_LimitingPos[0][2] = Unitree_DunkMotor_LimitingPos[0][2] + Dunkmotor_Contect_LimitingPos(2);
                }
                Unitree_DunkMotor_LimitingPos[0][0] = Unitree_DunkMotor_LimitingPos[0][0]/10;
                Unitree_DunkMotor_LimitingPos[0][1] = Unitree_DunkMotor_LimitingPos[0][1]/10;
                Unitree_DunkMotor_LimitingPos[0][2] = Unitree_DunkMotor_LimitingPos[0][2]/10;
            
        }

        if (Unitree_DunkMotor_LimitingPos[0][0] != 0)
        {
            Found_LimitingPos_Flag = 1;
        }

        //遥控器控制跳跃阶段
        switch ((uint32_t)MyRemote_Data.usr_right_knob)
        {
        case 0:
            my_Dunk_Status = DUNK_IDLE;
            break;
        case 4:
            my_Dunk_Status = DUNK_STAND_UP;
            break;
        case 8:
            my_Dunk_Status = DUNK_SQUAT;
            break;
        case 12:
            my_Dunk_Status = DUNK_JUMP_UP;
            break;
        default:
            break;
        }

        //状态控制
        if(Found_LimitingPos_Flag == 1)
        {
            switch (my_Dunk_Status)
            {
            case DUNK_IDLE:
            //Unitree_motor_0Torque();
                for (uint8_t i = 0; i < 3; i++)
                {
                    unitree_DunkMotor_t[i].cmd.Pos = Unitree_DunkMotor_LimitingPos[0][i];
                    if (unitree_DunkMotor_t[i].data.Pos >= Unitree_DunkMotor_LimitingPos[0][i])
                    {
                        unitree_DunkMotor_t[i].cmd.T = 0.2;
                        unitree_DunkMotor_t[i].cmd.K_P = 0.01;
                        unitree_DunkMotor_t[i].cmd.W = 0;
                        unitree_DunkMotor_t[i].cmd.K_W = 0.02;
                    }else{
                        
                        unitree_DunkMotor_t[i].cmd.T = 0.2;
                        unitree_DunkMotor_t[i].cmd.K_P = 0.01;
                        unitree_DunkMotor_t[i].cmd.W = 0.3;
                        unitree_DunkMotor_t[i].cmd.K_W = 0.02;
                    }
                }
                break;
            case DUNK_STAND_UP:
                for (uint8_t i = 0; i < 3; i++)
                {
                    unitree_DunkMotor_Tff[i] = 0.1;
                    
                    unitree_DunkMotor_t[i].cmd.Pos = Unitree_DunkMotor_LimitingPos[0][i] - Max_Extension_Angle;
                    unitree_DunkMotor_t[i].cmd.W = 0.4;
                    unitree_DunkMotor_t[i].cmd.K_W = 0.02;
                    unitree_DunkMotor_t[i].cmd.K_P = 0.06;
                    Unitree_motor_PosAndTor();
                }
                break;
            case DUNK_SQUAT:

                break;
            case DUNK_JUMP_UP :
                break;
            
            default:
                break;
            }
        
        }
        
        /* unitree_DunkMotor_posdes[0]=0;        //电机输出轴输出位置（-65051.18 ~ 65051.18  单位 rad）
        unitree_DunkMotor_kp[0] = 0;          //电机刚度系数/位置误差比例系数（0 ~ 25.599）
        unitree_DunkMotor_kw[0] = 0.02; */

        osDelay(1);
    }
    
}