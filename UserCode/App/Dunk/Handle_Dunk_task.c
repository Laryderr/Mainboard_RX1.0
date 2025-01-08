/**
 * @file Handle_Dunk_task.c
 * @author Lary (you@domain.com)
 * @brief 扣篮实现线程
 * @version 0.1
 * @date 2024-11-24
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention 腿向下蹬是正转，抬腿是反转
 * 
 */

#include "Unitree.h"
#include "Handle_Dunk_task.h"


Dunk_Task_t my_Dunk_Task_t;//定义扣篮任务结构体


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
 * @param 电机id
 */
void Unitree_motor_0Torque(uint8_t i)
{
        unitree_DunkMotor_t[i].cmd.T = 0;
        unitree_DunkMotor_t[i].cmd.K_P = 0;
        unitree_DunkMotor_t[i].cmd.W = 0;
        unitree_DunkMotor_t[i].cmd.K_W = 0.02;
}

/**
 * @brief PD控制器力位混合控制，加入前馈力矩
 * 
 */
void Unitree_motor_PosAndTor(void)
{
    for (uint8_t i = 0; i < 4; i++)
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
    for (uint8_t i = 0; i < 3; i++)
    {
            unitree_DunkMotor_t[i].cmd.Pos = 0;
            unitree_DunkMotor_t[i].cmd.K_P = 0;
            unitree_DunkMotor_t[i].cmd.K_W = 0;
            unitree_DunkMotor_t[i].cmd.T = -0.45;
            unitree_DunkMotor_t[i].cmd.W = 0;
    }
    return unitree_DunkMotor_t[i].data.Pos;

}

/**
 * @brief 跳跃抛球
 * 
 * @param this_status 
 */
void ThrowBall(void)
{
    if (unitree_DunkMotor_t[3].data.Pos <= my_Dunk_Task_t.Throwball_InitialPos - 2.1)
    {
        Unitree_motor_0Torque(3);
    }else{
        unitree_DunkMotor_t[3].cmd.Pos =0;
        unitree_DunkMotor_t[3].cmd.K_P = 0;
        unitree_DunkMotor_t[3].cmd.K_W = 0;
        unitree_DunkMotor_t[3].cmd.W = 0;
        unitree_DunkMotor_Tff[3] = -1.2;
        unitree_DunkMotor_t[3].cmd.T = unitree_DunkMotor_Tff[3] + unitree_DunkMotor_t[3].cmd.K_P * (unitree_DunkMotor_t[3].cmd.Pos - 
        unitree_DunkMotor_t[3].data.Pos) + unitree_DunkMotor_t[3].cmd.K_W * (unitree_DunkMotor_t[3].cmd.W - unitree_DunkMotor_t[3].data.W);
        osDelay(2);
    }

}


/** 
 * @brief 底盘Unitree跳跃电机控制线程
 * 
 */
void Handle_Dunk_Task(void *argument)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        unitree_DunkMotor_t[i].cmd.K_P = 0.02;
        unitree_DunkMotor_t[i].cmd.K_W = 0.02;
    }
    //标志初始化
    my_Dunk_Task_t.my_Dunk_Status = DUNK_IDLE;
    my_Dunk_Task_t.Time_TO_Jump = 0;
    my_Dunk_Task_t.my_StatusTransition_cnt = 0;
    my_Dunk_Task_t.Found_LimitingPos_Flag = 0;
    my_Dunk_Task_t.Foundthrow_InitialPos_Flag = 0;
    my_Dunk_Task_t.Jump_Completed_Flag = 0;

    for (; ;)
    {
        //检测抬腿极限位置
        if (my_Dunk_Task_t.Found_LimitingPos_Flag == 0)
        {
            //for (uint8_t i = 0; i < 4; i++)
            //{
                my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][0] =  Dunkmotor_Contect_LimitingPos(0);
                my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][1] =  Dunkmotor_Contect_LimitingPos(1);
                my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][2] =  Dunkmotor_Contect_LimitingPos(2);
            //}
            /*my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][0] = my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][0]/10;
            my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][1] = my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][1]/10;
            my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][2] = my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][2]/10;*/
            
        }

        //检测投球极限位置
        if (my_Dunk_Task_t.Foundthrow_InitialPos_Flag == 0)
        {
            unitree_DunkMotor_t[3].cmd.Pos = 0;
            unitree_DunkMotor_t[3].cmd.K_P = 0;
            unitree_DunkMotor_t[3].cmd.K_W = 0;
            unitree_DunkMotor_t[3].cmd.T = 0.2;
            unitree_DunkMotor_t[3].cmd.W = 0;
            for (uint8_t i = 0; i < 10; i++)
            {
                my_Dunk_Task_t.Throwball_InitialPos += unitree_DunkMotor_t[3].data.Pos;
            }
            my_Dunk_Task_t.Throwball_InitialPos = my_Dunk_Task_t.Throwball_InitialPos/10;

            if (my_Dunk_Task_t.Throwball_InitialPos != 0)
            {
                my_Dunk_Task_t.Foundthrow_InitialPos_Flag = 1;
            }
            
        }
        

        //检测是否成功读取三条腿极限位置
        if (my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][0] != 0)
        {
            if (my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][1] != 0)
            {
                if (my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][2] != 0)
                {
                    my_Dunk_Task_t.Found_LimitingPos_Flag = 1;
                }
            }
        }

        //遥控器控制跳跃阶段
        if(MyRemote_Data.btn_KnobR == 1)
        {
            my_Dunk_Task_t.my_Dunk_Status = DUNK_JUMP_UP;
        }
        if (MyRemote_Data.btn_KnobL == 1)
        {
            my_Dunk_Task_t.my_Dunk_Status = DUNK_IDLE;
        }
        

        //状态控制
        if(my_Dunk_Task_t.Found_LimitingPos_Flag == 1)
        {
            switch (my_Dunk_Task_t.my_Dunk_Status)
            {
            
            case DUNK_IDLE:
                //Unitree_motor_0Torque();
                for (uint8_t i = 0; i < 3; i++)
                {
                    unitree_DunkMotor_t[i].cmd.Pos = 0;
                    unitree_DunkMotor_t[i].cmd.K_P = 0;
                    unitree_DunkMotor_t[i].cmd.K_W = 0;
                    unitree_DunkMotor_t[i].cmd.T = -0.4;
                    unitree_DunkMotor_t[i].cmd.W = 0;
                    
                }
                unitree_DunkMotor_t[3].cmd.Pos = my_Dunk_Task_t.Throwball_InitialPos;
                unitree_DunkMotor_t[3].cmd.K_P = 0.01;
                unitree_DunkMotor_t[3].cmd.W = 0;
                unitree_DunkMotor_t[3].cmd.K_W = 0.03;
                unitree_DunkMotor_t[3].cmd.T = 0;

                my_Dunk_Task_t.Jump_Completed_Flag = 0;
                my_Dunk_Task_t.Time_TO_Jump = 0;
                break;
            case DUNK_JUMP_UP:
                //Unitree_motor_0Torque();
                //开始摆臂
                ThrowBall();
                //达到起跳时机
                /*if (unitree_DunkMotor_t[3].data.Pos <= my_Dunk_Task_t.Throwball_InitialPos - 0.2)
                {
                    my_Dunk_Task_t.Time_TO_Jump = 1;
                }
                
                //执行跳跃
                if(my_Dunk_Task_t.Time_TO_Jump == 1)
                {
                    for (uint8_t i = 0; i < 3; i++)
                    {
                        //达到跳跃伸腿极限
                        if(unitree_DunkMotor_t[i].data.Pos >= ( my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][i] + Max_Extension_Angle ))
                        {
                            my_Dunk_Task_t.Jump_Completed_Flag ++;

                            //开始收腿
                            unitree_DunkMotor_t[i].cmd.Pos = 0;
                            unitree_DunkMotor_t[i].cmd.K_P = 0;
                            unitree_DunkMotor_t[i].cmd.K_W = 0;
                            unitree_DunkMotor_t[i].cmd.T = -1.5;
                            unitree_DunkMotor_t[i].cmd.W = 0;

                            //完成收腿恢复正常扭矩
                            if (unitree_DunkMotor_t[i].data.Pos <= my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][i])
                            {
                                unitree_DunkMotor_t[i].cmd.T = -0.45;
                            }

                            //三条腿收腿完毕
                            if (my_Dunk_Task_t.Jump_Completed_Flag >= 3)
                            {
                                my_Dunk_Task_t.my_Dunk_Status = DUNK_IDLE;
                            }
                            
                            
                        }
                    }
                    
                    //未达到跳跃极限
                    if(my_Dunk_Task_t.Jump_Completed_Flag <= 3)
                    {
                        for (uint8_t i = 0; i < 3; i++)
                        {
                            unitree_DunkMotor_Tff[i] = 19;
                            unitree_DunkMotor_t[i].cmd.Pos = my_Dunk_Task_t.Unitree_DunkMotor_LimitingPos[0][i] + Max_Extension_Angle ;
                            unitree_DunkMotor_t[i].cmd.W = 2;
                            unitree_DunkMotor_t[i].cmd.K_P = 0.06;
                            unitree_DunkMotor_t[i].cmd.K_W = 0.06;
                            Unitree_motor_PosAndTor();
                        }
                    
                    }
                }*/
                break;
            default:
                break;
            }
        
        }
        
        osDelay(1);
    }
    
}