#include "lcd.h"
#include "touch.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>

int touchFlag = 0;
int touchData = -1;

int start = 0;
extern int imageIndex;
extern char *imagePaths[];

int error = -1;
int dataNum = 0;
extern int musicFlag;
extern int menuFlag;

void *threadFunc()
{

    //获取触摸屏数据
    while (1)
    {
        //开机加载完成后，开始读取数据
        if (touchFlag == 0 && start == 1) //当无其他触摸事件发生时，接收触摸屏数据并处理
        {
            touchData = getTouchData();

            //printf(" befor touchFlag = %d \n", touchFlag);
            touchFlag = 1;
            //printf(" after touchFlag = %d \n", touchFlag);
            printf("musicFlag = %d \n", musicFlag);
            printf("menuFlag = %d \n", menuFlag);
        }
        /*
        由于我们是丢弃数据，所以不需要while循环帮我读取数据，否则会丢失真正需要的数据。
        如果用的是getTouchData，那么便会卡在这里读取一个数据，但这读取到的数据会是已经是touchFlag=0时候的有效数据。
        */
    }
}

int main()
{
    //初始化LCD显示屏
    int res = lcdInit();
    if(-1 == res)
    {
        return -1;
    }
    //初始化触摸屏
    res = touchInit();
    if(-1 == res)
    {
        return -1;
    }

    //创建一个收集触摸屏数据线程
    pthread_t thread;
    res = pthread_create(&thread, NULL, threadFunc, NULL);


    //加载界面
    loading();

    //即将进入上滑解锁界面
    usleep(500000);

    start = 1; //允许读取数据

    //上滑解锁界面
    swipe();

    printf("上滑解锁通过\n");
    //锁屏
    lock();
    printf("锁屏通过\n");

    //等待0.5m后进入电子相册
    usleep(500000);
    lcdDrawBMP(0,0,imagePaths[imageIndex], 0);
    musicTools(0);

    printf("到达相册\n");
    //进入相册
    musicPhotoAlbum();

    //关闭触摸屏
    touchClose();
    //关闭LCD屏幕
    lcdClose();

    return 0;
}