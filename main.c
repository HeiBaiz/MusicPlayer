#include "display.h"
#include "touch.h"

#include <stdio.h>
#include <pthread.h>

#define URL_MAX 3
#define URL_MIN 1
#define SPE_MAX 3
#define SPE_MIN 1

int flag_special = 1;
int flag_thread = 0;
int password[4] ={1,2,3,4};

/************************************************
 * 功能：初始化背景、组件
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
    // 显示按钮
    lcdDrawRect(690,10,100,50,COLOR_RED);
    lcdDrawRect(690,420,100,50,COLOR_BLUE);
    lcdDrawRect(350,400,100,50,COLOR_PURPLE);
    flag_thread = 0;
}

/************************************************
 * 功能：开机动画
 * 参数：无
 * 返回值：无
*************************************************/
void *func() {
    char url[15];
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // 允许取消
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // 延迟取消
    while(1) {
        for(int i=1;i<20;i++){
            sprintf(url,"./g%d.bmp",i);
            lcdDrawBMP(url,0,325,165);
            usleep(1000*100);
        }
    }
}


int main() {
    // 初始化lcd显示屏
    int res = lcd_init();
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
/********************************************************************************************************************************* */
    // 过场动画
    lcd_brushBG(0);
    pthread_t thread;
    res = pthread_create(&thread, NULL, func, NULL);
    if (0 != res)
    {
        printf("thread create error!\n");
        return -1;
    }

    // 上划解锁
    while(getTouchData()!=13){
    }
    pthread_cancel(thread);
/*********************************************************************************************************************************** */
    //输入密码
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
            password0[i]=getTouchData();
            sprintf(url,"./suo%d.bmp",i+1);
            lcdDrawBMP(url,0,306,0);
            printf("password_%d=%d\n",i,password0[i]);
        }

        if(password0[0]==password[0]&&password0[1]==password[1]&&password0[2]==password[2]&&password0[3]==password[3]){
            break;
        }
        lcdDrawBMP("./error.bmp",0,360,0);
        usleep(100*1000);
        lcdDrawBMP("./suo.bmp",0,306,0);
    } while(1);

    int flag_url = 1;
    elementInit("./1.bmp");


    //获取触摸屏数据
    while (1)
    {
        switch (getTouchData())
        {
        case -1:
            printf("error!\n");
            return -1;
        // case 1:
        //     printf("点击了按键1\n");
        //     break;
        // case 2:
        //     printf("点击了按键2\n");
        //     break;
        // case 3:
        //     printf("点击了按键3\n");
        //     break;
        case 11:
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
            // elementInit(url);
            break;
        case 12:
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
            // elementInit(url);
            break;
        case 13:
            printf("上划\n");
            if(flag_special-- <= SPE_MIN){
                flag_special = SPE_MAX;
            }
            // elementInit(url);
            break;
        case 14:
            printf("下划\n");
            if(flag_special++ >= SPE_MAX){
                flag_special = SPE_MIN;
            }
            // elementInit(url);
            break;
        default:
            printf("未知操作！\n");
            break;
        }
    }


    pthread_join(thread, NULL);
    //关闭触摸屏
    touchClose();
    //关闭LCD屏幕
    lcd_close();
    return 0;
}
