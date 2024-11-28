#ifndef __HANDLE_DUNK_TASK_H
#define __HANDLE_DUNK_TASK_H
#ifdef __cplusplus
extern "C" {
#endif

#include "usermain.h"

#define Max_Extension_Angle 10.4497 ; //轮腿电机从抬腿到伸腿的最大角度

enum Dunk_Status{
    DUNK_IDLE = 0,
    DUNK_STAND_UP,  //站立
    DUNK_SQUAT,     //下蹲
    DUNK_JUMP_UP,   //跳起
};

extern float unitree_DunkMotor_LimitingPos[2][3]; // 0:抬腿极限位置，1：伸腿极限位置
extern bool Found_LimitingPos_Flag ;



//外部接口函数定义
void my_unitree_dunk_ctrl_TaskStart(void);

#ifdef __cplusplus
}
#endif
// C++
#endif // 