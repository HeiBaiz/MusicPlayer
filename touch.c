#include "touch.h"

int fd_touch;   //触摸屏设备文件描述

/************************************************
 * 功能：初始化触摸屏
 * 参数：无
 * 返回值：
 *      -1：失败
 *       0：成功
*************************************************/
int touchInit()
{
    //打开触摸屏设备文件/dev/input/event0
    fd_touch = open("/dev/input/event0",O_RDONLY);
    if (-1 == fd_touch)
    {
        perror("open touch error");
        return -1;
    }
    return 0;
}

/************************************************
 * 功能：关闭触摸屏
 * 参数：无
 * 返回值：无
*************************************************/
void touchClose()
{
    close(fd_touch);
}

/************************************************
 * 功能：获取触摸屏数据
 * 参数：
 *    lock：选择返回类
 * 返回值：
 *      -1：获取触摸屏数据失败
 *       0：未知操作
 *       1：点击了按键1
 *       2：点击了按键2
 *       3：点击了按键3
 *      11：上划
 *      12：下划
 *      13：左划
 *      14：右划
*************************************************/
int getTouchData(int lock0) {
    //定义一个输入事件结构体变量，用来保存读到的数据
    struct input_event ev;
    //循环读取触摸屏设备文件中的数据
    ssize_t res;
    int x0,y0,x,y;
    while (1) {
        res = read(fd_touch,&ev,sizeof(ev));
        // printf("res=%ld\n",res);
        if (-1 == res) {
            perror("read touch data error");
            return -1;
        }
        //分析处理数据
        // printf("type=0x%0x,code=0x%0x,value=%d\n",ev.type,ev.code,ev.value);
        if (ev.type == EV_ABS) {
            //绝对值事件
            if (ev.code == ABS_X) {
                //触摸点的坐标X值
                x = ev.value;
                //如果是黑色PCB板的屏幕，需要对触摸点的坐标进行等比缩放
                x = x*799/1023.0;
            } else if (ev.code == ABS_Y) {
                //触摸点的坐标Y值
                y = ev.value;
                //如果是黑色PCB板的屏幕，需要对触摸点的坐标进行等比缩放
                y = y*479/599.0;
            }
        } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
            //触摸屏点击事件
            if (ev.value == 1) {
                // 触摸屏被按下(手指接触触摸屏)
                // printf("pressed!\n");
                // 获取触摸点的起始坐标
                x0 = x;
                y0 = y;
            } else if (ev.value == 0) {
                // 触摸屏被释放(手指离开触摸屏)
                // printf("released!\n");
                // 判断是点击还是滑动
                if (abs(y-y0) < 50 && abs(x-x0) < 50) {
                    // 点击
                    // 判断点击的区域
                    if(lock0==0) {
                        // 锁屏响应
                        if (x >= 285 && x < 335 && y >= 220 && y < 270) {
                            // 点击了按键1
                            return 1;
                        } else if (x >= 375 && x < 425 && y >= 220 && y < 270) {
                            // 点击了按键2
                            return 2;
                        } else if (x >= 465 && x < 515 && y >= 220 && y < 270) {
                            // 点击了按键3
                            return 3;
                        } else if (x >= 285 && x < 335 && y >= 290 && y < 340) {
                            // 点击了按键4
                            return 4;
                        } else if (x >= 375 && x < 425 && y >= 290 && y < 340) {
                            // 点击了按键5
                            return 5;
                        } else if (x >= 465 && x < 515 && y >= 290 && y < 340) {
                            // 点击了按键6
                            return 6;
                        } else if (x >= 285 && x < 335 && y >= 360 && y < 410) {
                            // 点击了按键7
                            return 7;
                        } else if (x >= 375 && x < 425 && y >= 360 && y < 410) {
                            // 点击了按键8
                            return 8;
                        } else if (x >= 465 && x < 515 && y >= 360 && y < 410) {
                            // 点击了按键9
                            return 9;
                        } else if (x >= 375 && x < 425 && y >= 430 && y < 480) {
                            // 点击了按键0
                            return 0;
                        } else {
                            // 未知操作
                            return 10;
                        }
                    } else if (lock0==1) {
                        // 桌面响应
                        if (x>=100 && x<180 && y>=100 && y<180) {
                            printf("photo\n");
                            return 1;
                        } else if (x>=220 && x<300 && y>=100 && y<180) {
                            return 2;
                        }

                    }
                } else {
                    // 滑动，判断是水平滑动还是垂直滑动
                    x0-=x,y0-=y;    // x0>0 向左滑动，y0>0 向上滑动
                    if(lock0==0) {
                        if(y0>0) {
                            // 上划
                            return 3;
                        }
                    } else if (lock0==2) {
                        // 相册响应

                        if(abs(x0)>abs(y0)){
                            // 如果是水平滑动，判断是左划还是右划
                            if(x0>0) {
                                // 左划
                                return 1;
                            } else {
                                // 右划
                                return 2;
                            }
                            // 未知操作
                            return 10;
                        } else {
                            // 如果是垂直滑动，判断是上划还是下划
                            if(y0>0) {
                                // 上划
                                return 3;
                            } else {
                                // 下划
                                return 4;
                            }
                            // 未知操作
                            return 10;
                        }
                    }
                }
            }
        }
    }
}