#ifndef __HANDLE_DUNK_TASK_H
#define __HANDLE_DUNK_TASK_H
#ifdef __cplusplus
extern "C" {
#endif

#include "usermain.h"

#define Max_Extension_Angle 11.1  //轮腿电机从抬腿到伸腿的最大正转弧度

enum Dunk_Status{
    DUNK_IDLE = 0,
    DUNK_STAND_UP,  //站立
    DUNK_SQUAT,     //下蹲
    DUNK_JUMP_UP,   //跳起
};

/*enum ThrowBall_Status{
    ThrowBALL_UP = 0, //向上投球
    ThrowBALL_DOWN, //下放
};*/

typedef struct{
    enum Dunk_Status my_Dunk_Status; //当前的状态
    enum Dunk_Status mylast_Dunk_Status; //上一次的状态
    uint8_t my_StatusTransition_cnt; //状态切换计数器
    bool Found_LimitingPos_Flag; //是否找到底盘跳跃电机极限位置
    uint8_t Jump_Completed_Flag ;   //是否完成一次跳跃

    float Throwball_InitialPos; //投球电机初始位置
    bool Foundthrow_InitialPos_Flag; //是否找到投球电机初始位置
    bool Time_TO_Jump ; //投球臂是否转动到跳跃时机
    bool ThrowBall_Status ; //投球状态
    float Unitree_DunkMotor_LimitingPos[2][3]; // 0:抬腿极限位置，1：伸腿极限位置
}Dunk_Task_t;

extern Dunk_Task_t my_Dunk_Task_t;



//外部接口函数定义
void Handle_Dunk_TaskStart(void);

#ifdef __cplusplus
}
#endif
// C++
#endif // 