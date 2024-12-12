#include "touch.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdlib.h>


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
 * 参数：无
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
int getTouchData() //一次读两个事件，所以一次要放两个进来
{
    //定义一个输入事件结构体变量，用来保存读到的数据
    struct input_event ev;
    //循环读取触摸屏设备文件中的数据
    ssize_t res;
    int x0,y0,x,y;
    while (1)
    {
        res = read(fd_touch,&ev,sizeof(ev));
        if (-1 == res)
        {
            perror("read touch data error");
            return -1;
        }
        //分析处理数据
        // printf("type=0x%0x,code=0x%0x,value=%d\n",ev.type,ev.code,ev.value);
        if (ev.type == EV_ABS)
        {
            //绝对值事件
            if (ev.code == ABS_X)
            {
                //触摸点的坐标X值
                x = ev.value;
                //如果是黑色PCB板的屏幕，需要对触摸点的坐标进行等比缩放
                //x = x*799/1023.0;
            }
            else if (ev.code == ABS_Y)
            {
                //触摸点的坐标Y值
                y = ev.value;
                //如果是黑色PCB板的屏幕，需要对触摸点的坐标进行等比缩放
                //y = y*479/599.0;
            }
        }
        else if (ev.type == EV_KEY && ev.code == BTN_TOUCH)
        {
            //触摸屏点击事件
            if (ev.value == 1)
            {
                //触摸屏被按下(手指接触触摸屏)
                // printf("pressed!\n");
                //获取触摸点的起始坐标
                x0 = x;
                y0 = y;
            }
            else if (ev.value == 0)
            {
                //触摸屏被释放(手指离开触摸屏)
                // printf("released!\n");
                //判断是点击还是滑动
                if (abs(y-y0) < 50 && abs(x-x0) < 50)
                {
                    //点击
                    //判断点击的区域
                    if (x >= 275 && x < 335 && y >= 140 && y < 200)
                    {
                        //点击了按键1
                        return 1;
                    }
                    else if (x >= 375 && x < 435 && y >= 140 && y < 200)
                    {
                        //点击了按键2
                        return 2;
                    }
                    else if (x >= 475 && x < 535 && y >= 140 && y < 200)
                    {
                        //点击了按键3
                        return 3;
                    }
                    else if (x >= 275 && x < 335 && y >= 260 && y < 320)
                    {
                        //点击了按键3
                        return 4;
                    }
                    else if (x >= 375 && x < 435 && y >= 260 && y < 320)
                    {
                        //点击了按键3
                        return 5;
                    }
                    else if (x >= 475 && x < 535 && y >= 260 && y < 320)
                    {
                        //点击了按键3
                        return 6;
                    }
                    else if (x >= 275 && x < 335 && y >= 360 && y < 420)
                    {
                        //点击了按键3
                        return 7;
                    }
                    else if (x >= 375 && x < 435 && y >= 360 && y < 420)
                    {
                        //点击了按键3
                        return 8;
                    }
                    else if (x >= 475 && x < 535 && y >= 360 && y < 420)
                    {
                        //点击了按键3
                        return 9;
                    }
                    else if (x >=680 && x <= 800 && y >= 0 && y < 60)
                    {
                        //点击setting
                        return 21;
                    }
                    else if (x >=680 && x <= 800 && y >= 60 && y < 135)
                    {
                        //点击start or stop
                        return 22;
                    }
                    else if (x >=680 && x <= 800 && y >= 135 && y < 210)
                    {
                        //点击next
                        return 23;
                    }
                    else if (x >=680 && x <= 800 && y >= 210 && y <= 285)
                    {
                        //点击previous
                        return 24;
                    }
                    else if (x >=680 && x <= 800 && y >= 285 && y <= 360)
                    {
                        //点击自动播放按钮
                        return 25;
                    }
                    
                    else
                    {
                        //未知操作
                        return 0;
                    }
                }
                else
                {
                    //滑动，判断是水平滑动还是垂直滑动
                    if (abs(y - y0) >= abs(x - x0))
                    {
                        //垂直滑动
                        if (y0 - y < 0) //下滑
                        {
                            return 12;
                        }
                        else //上滑
                        {
                            return 11;
                        }
                    }
                    else
                    {
                        if (x0 - x < 0) //右滑
                        {
                            return 14;
                        }
                        else //左滑
                        {
                            return 13;
                        }
                    }

                }
            }
        }
    }
}