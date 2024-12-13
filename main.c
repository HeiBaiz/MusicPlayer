#include "display.h"
#include "touch.h"
#include "music.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define URL_MAX 3
#define URL_MIN 1
#define SPE_MAX 6
#define SPE_MIN 1

int flag_special = 1;
int flag_thread = 0;
int password[4] ={1,2,3,4};
int res,musicdata=-1;
extern int musicFlag,flag_j;
pthread_t thread;

/************************************************
 * 功能：初始化背景、相册组件
 * 参数：
 *      url：要打印的背景路径
 * 返回值：无
*************************************************/
void *elementInit(void *data) {
    flag_thread = 1;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // 允许取消
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); // 使用延迟取消
    // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // 延迟取消
    char *url = (char*)data;
     // 初始化背景图
    lcdDrawBMP(url,1,0,0);
    // 显示背景图
    randDisplay(0,0,flag_special);
    pthread_testcancel();
    flag_thread = 0;
}

/************************************************
 * 功能：开机动画
 * 参数：无
 * 返回值：无
*************************************************/
void *startupPicture() {
    char url[15];
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // 允许取消
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // 延迟取消
    while(1) {
        for(int i=1;i<31;i++) {
            sprintf(url,"./g%d.bmp",i);
            // printf("%s\n",url); //图片测试
            lcdDrawBMP(url,0,160,0);
            usleep(1000);
        }
    }
}

/************************************************
 * 功能：相册
 * 参数：无
 * 返回值：无
*************************************************/
int photo() {
    char url[15];
    int flag_url = 1;

    lcdDrawBMP("./1.bmp",0,0,0);
    // 相册功能-获取触摸屏数据
    while (1)
    {
        switch (getTouchData(2))
        {
        case -1:
            printf("error!\n");
            return -1;
        case 1:
            if(flag_thread) {
                pthread_cancel(thread);
                pthread_join(thread, NULL);
                printf("\n存在其它进程，已中断\n\n");
                flag_thread = 0;
            }
            flag_url--;
            if(flag_url == URL_MIN-1){
                flag_url = URL_MAX;
            }
            sprintf(url,"./%d.bmp",flag_url);
            printf("左划\n%s\n",url);
            res = pthread_create(&thread, NULL, elementInit, (void *)&url);
            if (0 != res)
            {
                printf("thread create error! error-%d\n",res);
                return -1;
            }
            break;
        case 2:
            if(flag_thread) {
                pthread_cancel(thread);
                pthread_join(thread, NULL);
                printf("\n存在其它进程，已中断\n\n");
                flag_thread = 0;
            }
            flag_url++;
            if(flag_url == URL_MAX+1){
                flag_url = URL_MIN;
            }
            sprintf(url,"./%d.bmp",flag_url);
            printf("右划\n%s\n",url);
            res = pthread_create(&thread, NULL, elementInit, (void *)&url);
            if (0 != res)
            {
                printf("thread create error! error-%d\n",res);
                return -1;
            }
            break;
        case 3:
            printf("上划\n");
            // if(flag_special-- <= SPE_MIN){
            //     flag_special = SPE_MAX;
            // }
            if(flag_special++ >= SPE_MAX){
                flag_special = SPE_MIN;
            }
            break;
        case 4:
            printf("下划\n");
            return 0;
        default:
            printf("未知操作！\n");
            break;
        }
    }
}

/************************************************
 * 功能：音乐读数据
 * 参数：无
 * 返回值：无
*************************************************/
void *musicData() {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // 允许取消
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); // 使用延迟取消
    while(1){
        musicdata =  getTouchData(3);
        printf("%d\n",musicdata);
        pthread_testcancel();
    }
}

/************************************************
 * 功能：音乐
 * 参数：无
 * 返回值：无
*************************************************/
int music() {
    int status;
    lcdDrawBMP("./1.bmp",0,0,0);
    lcdDrawBMP("./next.bmp",0,475,400);
    lcdDrawBMP("./previous.bmp",0,275,400);
    lcdDrawBMP("./start.bmp",0,375,400);
    printf("music\n");
    pid_t pid = -2;
    res = pthread_create(&thread, NULL, musicData, NULL);
    if (0 != res)
    {
        printf("thread create error! error-%d\n",res);
        return -1;
    }
    while(1){
        switch (musicdata)
        {
        case 1:
            musicPlayer(&pid,1);
            lcdDrawBMP("./stop.bmp",0,375,400);
            musicdata=-1;
            break;
        case 2:
            musicPlayer(&pid,2);
            musicdata=-1;
            lcdDrawBMP("./stop.bmp",0,375,400);
            lcdDrawBMP("./next.bmp",0,475,400);
            lcdDrawBMP("./previous.bmp",0,275,400);
            break;
        case 3:
            musicPlayer(&pid,3);
            musicdata=-1;
            lcdDrawBMP("./stop.bmp",0,375,400);
            lcdDrawBMP("./next.bmp",0,475,400);
            lcdDrawBMP("./previous.bmp",0,275,400);
            break;
        case 4:
            if(musicFlag==0){
                kill(pid,SIGCONT);
            }
            kill(pid,SIGTERM);
            pthread_cancel(thread);
            flag_j=1;
            musicdata=-1;
            musicFlag=0;
            return 0;
        default:
            musicdata=-1;
            break;
        }
        if(pid == waitpid(pid, &status, WNOHANG)) {
            pid = -2;
            musicFlag=0;
            // printf("musicFlag=%d\n",musicFlag);
        }
        if(musicFlag==0) {
            // printf("暂停\n");
            lcdDrawBMP("./start.bmp",0,375,400);
        }
    }
}

/************************************************
 * 功能：初始化组件
 * 参数：无
 * 返回值：
 *          -1：错误
*************************************************/
int Init() {
    // 初始化lcd显示屏
    res = lcd_init();
    if(res == -1) {
        perror("lcd init error!\n");
        return -1;
    }

    // 初始化触摸屏
    res = touchInit();
    if(res == -1) {
        perror("touch init error!\n");
        return -1;
    }
}

/************************************************
 * 功能：开机动画
 * 参数：无
 * 返回值：
 *          -1：错误
*************************************************/
int Start() {
    // 过场动画
    lcd_brushBG(0);
    res = pthread_create(&thread, NULL, startupPicture, NULL);
    if (0 != res)
    {
        printf("thread create error!\n");
        return -1;
    }
    sleep(5);
    // 结束开机动画
    pthread_cancel(thread);
    
    // 刷新背景色
    lcd_brushBG(0);

    lcdDrawBMP("./suopin.bmp",0,0,0);
    // 上划解锁
    while(getTouchData(0)!=3){
    }
}

/************************************************
 * 功能：锁屏
 * 参数：无
 * 返回值：无
*************************************************/
void Lock() {
    // 显示密码表盘
    char url[15];
    lcd_brushBG(0);
    lcdDrawBMP("./1.bmp",0,0,0);
    for(int i=0;i<9;i++) {
        sprintf(url,"./key%d.bmp",i+1);
        lcdDrawBMP(url,0,285 + i%3*90,220 + (i/3)*70);
    }
    lcdDrawBMP("./key0.bmp",0,375,430);
    lcdDrawBMP("./suo.bmp",0,306,0);
    int password0[4];
    // 输入和验证密码
    do {
        for(int i=0;i<4;i++){
            // 确保password0[i]里保存的是有效数据
            do {
                password0[i]=getTouchData(0);
                sprintf(url,"./suo%d.bmp",i+1);
            } while (password0[i]<0||password0[i]>9);
            // 更新密码输入位数图片
            lcdDrawBMP(url,0,306,0);
            // printf("password_%d=%d\n",i,password0[i]);
        }

        // 对比输入和存储的密码，相等就退出while循环
        if(password0[0]==password[0]&&password0[1]==password[1]
        &&password0[2]==password[2]&&password0[3]==password[3]) {
            break;
        }

        // 显示密码错误图片
        lcdDrawBMP("./error.bmp",0,360,0);
        usleep(100*1000);
        lcdDrawBMP("./suo.bmp",0,306,0);
    } while(1);
}

int main() {
    // 初始化组件
    if(Init()==-1)
        return -1;

    // 开机动画
    if(Start()==-1)
        return -1;

    // 锁屏
    Lock();
    
    // 桌面循环
    while(1) {
        // 桌面背景
        elementInit("./1.bmp");
        // 应用图标
        lcdDrawBMP("./photo.bmp",0,100,100);
        lcdDrawBMP("./music.bmp",0,220,100);

        switch (getTouchData(1)) {
            case 1:
                lcd_brushBG(0);
                photo();
                pthread_join(thread, NULL);
                break;
            case 2:
                lcd_brushBG(0);
                music();
                pthread_join(thread, NULL);
                break;
            
            default:
                break;
        }
    }
    
    

    //关闭触摸屏
    touchClose();
    //关闭LCD屏幕
    lcd_close();
    return 0;
}
