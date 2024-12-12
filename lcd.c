#include "lcd.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>



int fb_size = 800*480*4;    //帧缓冲设备文件的大小
int fd_lcd; //帧缓冲设备文件的文件描述符
unsigned int *pmap; //映射区首地址



unsigned char a,r,g,b;

short MS,depth; //魔数、色深
int offset,w,h; //像素数组文件偏移量、宽度、高度
int full_bytes;

char *imagePaths[] = 
{
    "./1.bmp",
    "./2.bmp",
    "./3.bmp"
};

char *lockImagePaths[] = 
{
    "./atri1.bmp",
    "./atri2.bmp",
};

char *loadingImage[] = {
    "loading1.bmp",
    "loading2.bmp",
    "loading3.bmp"
};

char *lockImage[] = {
    "suo0.bmp",
    "suo1.bmp",
    "suo2.bmp",
    "suo3.bmp",
    "suo4.bmp"
};

char *buttonImage[] = {
    "button1.bmp",
    "button2.bmp",
    "button3.bmp",
    "button4.bmp",
    "button5.bmp",
    "button6.bmp",
    "button7.bmp",
    "button8.bmp",
    "button9.bmp",
    "button0.bmp"
};

char *musicToolsImage[] = {
    "menu.bmp",
    "tools1.bmp",
    "tools2.bmp",
    "tools3.bmp",
    "tools4.bmp"
};


char *musicPaths[] = {
    "/home/ys/pm.mp3",
    "/home/ys/donadona.mp3",
    "/home/ys/vergil.mp3"
};


char password[] = {"1234"};

int block[] = {1, 0, 2}; //特效区块

int kind = 0;
int imageIndex = 0;
int loadingImageFlag = 0;
int lockFlag = 0;
int lockStatus = 1;
int swipeUpFlag = 1;
int lockImageFlag = 0;
int v = 0;
int menuFlag = 0;
int musicFlag = 0;
int musicIndex = 0;
int autoMusicFlag = 0;
int status;

pid_t pid = -2;
pid_t temp;

extern int touchData;
extern int touchFlag;

/************************************************
 * 功能：初始化LCD屏幕
 * 参数：无
 * 返回值：
 *      -1：失败
 *       0：成功
*************************************************/
int lcdInit()
{
    // 打开帧缓冲设备文件(/dev/fb0)
    fd_lcd = open("/dev/fb0",O_RDWR);
    if (-1 == fd_lcd)
    {
        perror("open lcd error");
        return -1;
    }
    //映射
    pmap = mmap(NULL, fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_lcd, 0);
    if (MAP_FAILED == pmap)
    {
        perror("mmap error");
        close(fd_lcd);
        return -1;
    }
    return 0;
}

/************************************************
 * 功能：关闭LCD屏幕
 * 参数：无
 * 返回值：无
*************************************************/
void lcdClose()
{
    //解除映射
    munmap(pmap,fb_size);
    // 关闭帧缓冲设备文件
    close(fd_lcd);
}

/************************************************
 * 功能：刷背景色
 * 参数：
 *      color：要显示的背景颜色
 * 返回值：无
*************************************************/
void lcdBrushBG(unsigned int color)
{
    for (int y = 0; y < 480; y++)
    {
        for (int x = 0; x < 800; x++)
        {
            *(pmap+y*800+x) = color;
        }
    }
}

/************************************************
 * 功能：在指定的位置画像素点
 * 参数：
 *      x,y：像素点的坐标
 *      color：像素点的颜色值
 * 返回值：无
*************************************************/
void lcdDrawPoint(int x,int y,unsigned int color)
{
    if (x >= 0 && x < 800 && y >= 0 && y < 480)
    {
        *(pmap+y*800+x) = color;
    }
}

/************************************************
 * 功能：在指定的位置画矩形(实心)
 * 参数：
 *      x0,y0：矩形显示的位置(左上顶点)
 *      w,h：矩形的宽度和高度
 *      color：矩形的填充颜色
 * 返回值：无
*************************************************/
void lcdDrawRect(int x0,int y0,int w,int h,unsigned int color)
{
    for (int y = y0; y < y0+h; y++)
    {
        for (int x = x0; x < x0+w; x++)
        {
            lcdDrawPoint(x,y,color);
        }
    }
}

/************************************************
 * 功能：在指定的位置画圆形(实心)
 * 参数：
 *      x0,y0：圆心的坐标
 *      r：半径
 *      color：圆形的填充颜色
 * 返回值：无
*************************************************/
void lcdDrawCircle(int x0,int y0,int r,unsigned int color)
{
    for (int y = y0-r; y < y0+r; y++)
    {
        for (int x = x0-r; x < x0+r; x++)
        {
            if ((y-y0)*(y-y0)+(x-x0)*(x-x0) <= r*r)
            {
                //是圆内的点，显示
                lcdDrawPoint(x,y,color);
            }
        } 
    }
}


void bmpFun(unsigned char *p, int kind, int x0, int y0)
{
    unsigned char *pnew;
    switch(kind)
    {
        case 0:
            for (int y = 0; y < abs(h); y++)
            {
                for (int x = 0; x < w; x++)
                {
                    b = *p++;
                    g = *p++;
                    r = *p++;
                    if (depth == 24)
                    {
                        a = 0;
                    }
                    else if(depth == 32)
                    {
                        a = *p++;
                    }
                    unsigned int color = a << 24 | r << 16 | g << 8 | b;
                    //将像素点显示到屏幕上去
                    if (h > 0)
                    {
                        //从下往上显示
                        lcdDrawPoint(x+x0,h-1-y+y0,color);
                    }
                    else
                    {
                        //从上往下显示
                        lcdDrawPoint(x+x0,y+y0,color);
                    }
                }
                p += full_bytes;
            }
            break;

        case 1:
            for(int k = 0; k < 3; k++)
            {
                pnew = p + 160 * block[k] * (w*depth/8+full_bytes); 
                for(int y = 0 + 160 * block[k]; y < 160 + 160 * block[k] ; y++)
                {

                    for(int x = 0; x < w; x++)
                    {
                        //获取一个像素点的4个颜色分量
                        b = *pnew++;
                        g = *pnew++;
                        r = *pnew++;
                        if (depth == 24)
                        {
                            //是24位的bmp图片，没有透明度，需要手动赋值
                            a = 0;
                        }
                        else if(depth == 32)
                        {
                            //是32位的bmp图片，有透明度，直接获取
                            a = *pnew++;
                        }
                        unsigned int color = a << 24 | r << 16 | g << 8 | b;
                        //将像素点显示到屏幕上去
                        if (h > 0) 
                        {
                            //从下往上显示
                            lcdDrawPoint(x+x0,h-1-y+y0,color);
                        }
                        else
                        {
                            //从上往下显示
                            lcdDrawPoint(x+x0,y+y0,color);
                        }
                    }
                }
                usleep(62500);
            }
            break;
        
        case 2:
            for(int k = 0; k < 3; k++)
            {
                pnew = p + 160 * block[k] * 4; 
                for(int y = 0 + 160 * block[k]; y < 160 + 160 * block[k] ; y++)
                {

                    for(int x = 0; x < w; x++)
                    {
                        //获取一个像素点的4个颜色分量
                        b = *pnew++;
                        g = *pnew++;
                        r = *pnew++;
                        if (depth == 24)
                        {
                            //是24位的bmp图片，没有透明度，需要手动赋值
                            a = 0;
                        }
                        else if(depth == 32)
                        {
                            //是32位的bmp图片，有透明度，直接获取
                            a = *pnew++;
                        }
                        unsigned int color = a << 24 | r << 16 | g << 8 | b;
                        //将像素点显示到屏幕上去
                        if (h > 0) 
                        {
                            //从下往上显示
                            lcdDrawPoint(x+x0,h-1-y+y0,color);
                        }
                        else
                        {
                            //从上往下显示
                            lcdDrawPoint(x+x0,y+y0,color);
                        }
                    }
                }
                usleep(80000);
            }
            for (int y = 0; y < abs(h); y++)
            {
                for (int x = 0; x < w; x++)
                {
                    //获取一个像素点的4个颜色分量
                    b = *p++;
                    g = *p++;
                    r = *p++;
                    if (depth == 24)
                    {
                        //是24位的bmp图片，没有透明度，需要手动赋值
                        a = 0;
                    }
                    else if(depth == 32)
                    {
                        //是32位的bmp图片，有透明度，直接获取
                        a = *p++;
                    }
                    unsigned int color = a << 24 | r << 16 | g << 8 | b;
                    //将像素点显示到屏幕上去
                    if (h > 0)
                    {
                        //从下往上显示
                        lcdDrawPoint(x+x0,h-1-y+y0,color);
                    }
                    else
                    {
                        //从上往下显示
                        lcdDrawPoint(x+x0,y+y0,color);
                    }
                }
                //每显示完一行像素点，就需要跳过后面的填充字节
                p += full_bytes;
            }

            break;

        case 3:

            for(int y = (abs(h) - 1) / 2; y >= 0 ; y--)
            {
                for(int k = 0; k < 2; k++)
                {
                    pnew = p + (w*depth/8+full_bytes) * y;
                    for(int x = 0; x < w; x++)
                    {
                        //获取一个像素点的4个颜色分量
                        b = *pnew++;
                        g = *pnew++;
                        r = *pnew++;
                        if (depth == 24)
                        {
                            //是24位的bmp图片，没有透明度，需要手动赋值
                            a = 0;
                        }
                        else if(depth == 32)
                        {
                            //是32位的bmp图片，有透明度，直接获取
                            a = *pnew++;
                        }
                        unsigned int color = a << 24 | r << 16 | g << 8 | b;
                        //将像素点显示到屏幕上去
                        if (h > 0)
                        {
                            //从下往上显示
                            lcdDrawPoint(x+x0,h-1-y+y0,color);
                        }
                        else
                        {
                            //从上往下显示
                            lcdDrawPoint(x+x0,y+y0,color);
                        }
                    }

                    pnew = p + (w*depth/8+full_bytes) * (479 - y);
                    for(int x = 0; x < w; x++)
                    {
                        //获取一个像素点的4个颜色分量
                        b = *pnew++;
                        g = *pnew++;
                        r = *pnew++;
                        if (depth == 24)
                        {
                            //是24位的bmp图片，没有透明度，需要手动赋值
                            a = 0;
                        }
                        else if(depth == 32)
                        {
                            //是32位的bmp图片，有透明度，直接获取
                            a = *pnew++;
                        }
                        unsigned int color = a << 24 | r << 16 | g << 8 | b;
                        //将像素点显示到屏幕上去
                        if (h > 0)
                        {
                            //从下往上显示
                            lcdDrawPoint(x+x0,h-1-(479 - y)+y0,color);
                        }
                        else
                        {
                            //从上往下显示
                            lcdDrawPoint(x+x0,y+y0,color);
                        }
                    }
                }
                usleep(3000);
            }
    }
    
}

/************************************************
 * 功能：在指定的位置显示图片
 * 参数：
 *      x0,y0：图片显示的位置(左上顶点的坐标)
 *      pic_path：要显示的图片的路径名
 * 返回值：无
*************************************************/
void lcdDrawBMP(int x0,int y0,const char *pic_path, int kind)
{
    //打开图片文件
    int fd_pic = open(pic_path,O_RDONLY);
    if (-1 == fd_pic)
    {
        perror("open pic error");
        return;
    }
    //读取图片重要数据(魔数、像素数组的文件偏移量、宽度、高度、色深)
    
    read(fd_pic,&MS,2);
    if (0x4D42 != MS)
    {
        printf("this pic is not bmp!\n");
        close(fd_pic);
        return;
    }
    lseek(fd_pic,0x0A,SEEK_SET);
    read(fd_pic,&offset,4);
    lseek(fd_pic,0x12,SEEK_SET);
    read(fd_pic,&w,4);
    read(fd_pic,&h,4);
    lseek(fd_pic,0x1C,SEEK_SET);
    read(fd_pic,&depth,2);
    // printf("MS=0x%X,offset=0x%X,w=%d,h=%d,depth=%d\n",MS,offset,w,h,depth);
    //读取像素数组数据(颜色值)
    full_bytes = (4-(w*depth/8)%4)%4;   //每一行的填充字节数 = (4-一行像素点颜色值总字节数%4)%4
    int buf_size = (w*depth/8+full_bytes)*abs(h);    //像素数组大小 = (一行像素点颜色值的总字节数+填充字节数)*行数
    unsigned char color_buf[buf_size];  //像素数组数据
    lseek(fd_pic,offset,SEEK_SET);
    read(fd_pic,color_buf,buf_size);
    //遍历整个图片，将每个像素点的颜色值取出来，按照argb的顺序重新排列
    unsigned char *p = color_buf;
    
    bmpFun(p, kind, x0 ,y0);



    //关闭图片文件
    close(fd_pic);
}


void lcdBootDraw(int x0, int y0, int num, int kind)
{
    char pic[50] = {};
    lcdDrawBMP(x0 - 170, y0 + 260, loadingImage[loadingImageFlag++ % 3], kind);
    for(int i = 1; i <= num; i++)
    {
        //白子跳动图标
        sprintf(pic, "open%d.bmp", i);
        lcdDrawBMP(x0, y0, pic, kind);

        if(i % 3 == 0)
        {
            //加载图标继续加载
            lcdDrawBMP(x0 - 170, y0 + 260, loadingImage[loadingImageFlag++ % 3], kind);
        }
        usleep(125000);
    }
    
}


void loading()
{
    
    //显示黑色背景图
    lcdBrushBG(0);

    for(int i = 0; i < 2; i++)
    {
        //开机动画
        lcdBootDraw(300, 60, 18, 0);
    }
    
    lcdDrawBMP(275,50, "./finish.bmp", 0);
}

void swipe()
{
    //上滑解锁界面
    lcdDrawBMP(0, 0 ,"swipeImage.bmp", 0);
    
    while(1) //等待上滑
    {
        if(touchFlag == 1) //接收到了数据
        {
            //printf("touchData = %d \n", touchData);
            if(11 == touchData) //上滑，解除循环
            {
                touchData = -1; //清空数据
                touchFlag = 0; //数据处理完毕
                break;
            }
            else
            {
                touchData = -1; //清空数据
                touchFlag = 0; //数据处理完毕
            }
            
        }
    }
}

void lock()
{
    //黑色背景
    lcdBrushBG(0);

    //密码个数显示一次
    lcdDrawBMP(310, 10, lockImage[lockFlag], 0);

    int k = 0;

    //按键打印

    for(int y = 0; y< 3; y++)
    {
        for(int x = 0; x < 3; x++)
        {
            lcdDrawBMP(275 + x * 100, 140 + y * 120, buttonImage[k++], 0);
        }
    }

    //锁住
    while(lockStatus)
    {

        if(touchFlag == 1)
        {
            //检测触摸屏数据

            //printf("touchData = %d \n", touchData);
            
            switch (touchData)
            {
            case -1:
                printf("error!\n");
                return ;
            case 1:
                printf("点击了按键1\n");
                password[lockFlag++] = '1';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0); //更新密码图片
                break;
            case 2:
                printf("点击了按键2\n");
                password[lockFlag++] = '2';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);
                break;
            case 3:
                printf("点击了按键3\n");
                password[lockFlag++] = '3';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);
                break;
            case 4:
                printf("点击了按键4\n");
                password[lockFlag++] = '4';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);
                break;
            case 5:
                printf("点击了按键5\n");
                password[lockFlag++] = '5';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);
                break;
            case 6:
                printf("点击了按键6\n");
                password[lockFlag++] = '6';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);
                break;
            case 7:
                printf("点击了按键7\n");
                password[lockFlag++] = '7';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);
                break;
            case 8:
                printf("点击了按键8\n");
                password[lockFlag++] = '8';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);
                break;
            case 9:
                printf("点击了按键9\n");
                password[lockFlag++] = '9';
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);
                break;

            case 13:
                //printf("左滑\n");
                
                lcdDrawBMP(0, 0, lockImagePaths[lockImageFlag = ++lockImageFlag % 2], 1);

                //密码个数显示一次
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);

                //按键打印
                for(int y = 0; y< 3; y++)
                {
                    for(int x = 0; x < 3; x++)
                    {
                        lcdDrawBMP(275 + x * 100, 140 + y * 120, buttonImage[v++], 0);
                    }
                }

                //用于重置按钮的序号
                v = 0;
                break;

            case 14:
                //printf("右滑\n");
                
                lcdDrawBMP(0, 0, lockImagePaths[lockImageFlag = --lockImageFlag < 0 ? 1 : 0], 1);

                //密码个数显示一次
                lcdDrawBMP(310, 10, lockImage[lockFlag], 0);

                //按键打印
                for(int y = 0; y< 3; y++)
                {
                    for(int x = 0; x < 3; x++)
                    {
                        lcdDrawBMP(275 + x * 100, 140 + y * 120, buttonImage[v++], 0);
                    }
                }
                v = 0;
                break;
            default:
                printf("未知操作！\n");
                break;
            }

            //printf("lockFlag = %d ", lockFlag);

            //数据处理完成
            touchData = -1;


            //进行循环的判断，判断是否离开循环
            if (lockFlag == 4)
            {
                printf("password = %s\n", password);
                for (int i = 0; i < 4; i++)
                {
                    if (password[i] != '1' + i)
                    {
                        printf("password[%d] = %c \n", i, password[i]);
                        printf("%c \n", password[i]);
                        lockFlag = 0;
                        usleep(250000);
                        lcdDrawBMP(305, 10, "./error.bmp", 0);
                        break;
                    }
                    if (i == 3)
                    {
                        //结束循环
                        lockStatus = 0;
                    }
                }
            }
            touchFlag = 0;
        }

    }
}


void musicTools(int num)
{
    lcdDrawBMP(740, 5, musicToolsImage[num], 0);
}


void createFork()
{
    pid = fork();
    if (-1 == pid)
    {
        perror("fork error");
        return;
    }
    else if (pid == 0)
    {
    //让子进程去播放音乐
    int res = execl("/usr/bin/madplay", "madplay", musicPaths[musicIndex], NULL);
    if (res == -1)
    {
        perror("execl error");
        return;
    }
    }
}



void musicPhotoAlbum()
{
    musicTools(menuFlag);

    while(1)
    {

        //音乐自动播放功能

        //两种可能，音乐还在播放，音乐播放完了，但还没检测进程状态
        //检查子进程是否结束函数，如果pid != -2代表曾经播放过音乐或者正在播放，所以需要检查歌曲是否结束
        // 已结束 -- pid赋值为-2
        // 未结束 -- 不做操作
        if (pid != -2)
        {
            temp = pid;
            pid = waitpid(temp, &status, WNOHANG);
            if (pid == temp) //歌曲已结束
            {
                pid = -2; //pid恢复默认值

                //刷新歌曲工具栏
                //音乐结束
                musicFlag = 0;
                if (menuFlag != 0 && autoMusicFlag == 0)
                {
                    menuFlag = 1;
                    lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                    musicTools(menuFlag);
                }
                else if (menuFlag != 0 && autoMusicFlag == 1)
                {
                    menuFlag = 2;
                    lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                    musicTools(menuFlag);
                }

            }
            else //歌曲未结束
            {
                pid = temp; //继续保持当前子进程pid
            }
        }
        
        //首先检测pid是否为-2，检测是否在播放音乐，若是没有播放过检查autoMusicFlag是否为1
        // autoMusicFlag == 1  ----创建子进程播放音乐
        // autoMusicFlag == 0  ----不做任何事
        if (pid == -2 && autoMusicFlag == 1)
        {
            musicIndex = ++musicIndex % 3; //先让歌曲预备到下一首
            //自动播放音乐
            createFork();
            //刷新歌曲工具栏
            musicFlag = 1;
            if (menuFlag != 0)
            {
                menuFlag = 4;
                lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                musicTools(menuFlag);
            }
        }

        if (touchFlag == 1)
        {
            //printf("touchData = %d \n", touchData);
            switch (touchData)
            {
                 case 11:
                //printf("上滑\n");
                kind = ++kind % 4;
                break;
            case 12:
                //printf("下滑\n");
                kind = --kind < 0 ? 3 : kind % 4;
                break;
            case 13:
                //printf("左滑\n");
                
                imageIndex = ++imageIndex % 3;
                lcdDrawBMP(0, 0, imagePaths[imageIndex], kind);

                musicTools(menuFlag); //音乐菜单栏
                //printf("imageIndex：%d ", imageIndex); 
                break;
            case 14:
                //printf("右滑\n");
                imageIndex = --imageIndex < 0 ? 2 : imageIndex % 3;
                lcdDrawBMP(0, 0, imagePaths[imageIndex], kind);

                musicTools(menuFlag); //音乐菜单栏
                //printf("imageIndex：%d ", imageIndex);
                break;
            case 21:
                //menu
                if (menuFlag == 0)
                {
                    if (musicFlag == 0)
                    {
                        if (autoMusicFlag == 0)
                        {
                            menuFlag = 1;
                            musicTools(menuFlag);
                        }
                        else
                        {
                            menuFlag = 2;
                            musicTools(menuFlag);
                        }
                    }
                    
                    else
                    {
                        if (autoMusicFlag == 0)
                        {
                            menuFlag = 3;
                            musicTools(menuFlag);
                        }
                        else
                        {
                            menuFlag = 4;
                            musicTools(menuFlag);
                        }
                    }
                }
                else
                {
                    menuFlag = 0;
                    lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                    musicTools(menuFlag);
                }
                break;
            
            case 22:
                //start or stop

                // temp = pid;
                // pid = waitpid(temp, &status, WNOHANG); //检测子进程是否结束，并且不阻塞父进程
                // if (pid == temp) //创建的进程已经结束了
                // {
                //     pid = -2;//回到初始数值
                // }

                /*这里的问题是，若是子进程没有结束，那么pid就会被waitpid函数赋值为0，使得丢失子进程号导致错误*/
                

                if (musicFlag == 0 && pid == -2 && menuFlag != 0) //menuFLag != 0 是为了当菜单栏未显示出来的时候不能点击其他功能按钮
                {
                    //创建子进程播放音乐

                    createFork();

                    musicFlag = 1;
                    if (menuFlag != 0)
                    {
                        if (autoMusicFlag == 0)
                        {
                            menuFlag = 3;
                            lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                            musicTools(menuFlag);
                        }
                        else
                        {
                            menuFlag = 4;
                            lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                            musicTools(menuFlag);
                        }
                        
                    }

                    //kill(pid, SIGTERM);

                }
                else if (musicFlag == 1 && pid != -2 && menuFlag != 0) 
                {
                    // //检测子进程是否结束，防止歌曲播放结束后，pid没有更新，导致以为一直有一个进程在放歌
                    // temp = pid;
                    // pid = waitpid(temp, &status, WNOHANG); //检测子进程是否结束，并且不阻塞父进程
                    // if (pid == temp) //创建的进程已经结束了
                    // {
                    //     pid = -2;//回到初始数值


                    //     //音乐结束
                    //     musicFlag = 0;
                    //     if (menuFlag != 0)
                    //     {
                    //         menuFlag = 1;
                    //         lcdDrawBMP(0,0,imagePaths[imageIndex], 0);
                    //         musicTools(menuFlag);
                    //     }
                    // }
                    //else //进程未结束，并且歌曲还在播放，所以需要暂停子进程
                    //{
                    //    pid = temp; //归还一下进程号

                        //暂停子进程
                        kill(pid, SIGSTOP);
                        printf("子进程暂停了\n");
                        musicFlag = 0;
                        if (menuFlag != 0)
                        {
                            if (autoMusicFlag == 0)
                            {
                                menuFlag = 1;
                                lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                                musicTools(menuFlag);
                            }
                            else
                            {
                                menuFlag = 2;
                                lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                                musicTools(menuFlag);
                            }
                        }
                    //}
                    
                }
                else if (musicFlag == 0 && pid != -2 && menuFlag != 0) //进程被暂停了，现在启动进程
                {
                    kill(pid, SIGCONT); //启动子进程
                    printf("子进程重新启动了\n");
                    musicFlag = 1;
                    if (menuFlag != 0)
                    {
                        if (autoMusicFlag == 0)
                        {
                            menuFlag = 3;
                            lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                            musicTools(menuFlag);
                        }
                        else
                        {
                            menuFlag = 4;
                            lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                            musicTools(menuFlag);
                        }
                        
                    }
                }
                break;
                
            case 23:
                //下一首
                if (menuFlag != 0)
                {
                    musicIndex = ++musicIndex % 3;
                printf("musicIndex = %d \n", musicIndex);
                if (pid != 2) //两种可能，音乐进程还没结束，音乐播放完了，但还没检测进程状态
                {
                    // //检测子进程是否结束
                    // temp = pid;
                    // pid = waitpid(temp, &status, WNOHANG); 
                    // if (pid == temp) //子进程结束
                    // {
                    //     //创建新的子进程
                        
                    //     createFork();
                    // }
                    // else //进程未结束
                    // {
                    //     pid = temp; 

                        //结束子进程
                        kill(pid, SIGTERM);
                        printf("子进程强制结束了\n");

                        //创建新的子进程
                        
                        createFork();
                    //}
                }
                
                //这里的话，没进入if条件，即没有音乐被暂停或者被播放或播放过，单纯切换到下一首歌并且不播放
                }

                break;
            case 24:
                if (menuFlag != 0)
                {
                    //上一首
                    musicIndex = --musicIndex < 0 ? 2 : musicIndex % 3;
                    printf("musicIndex = %d \n", musicIndex);
                    if (pid != 2) //两种可能，音乐进程还没结束，音乐播放完了，但还没检测进程状态
                    {
                        // //检测子进程是否结束
                        // temp = pid;
                        // pid = waitpid(temp, &status, WNOHANG); 
                        // if (pid == temp) //子进程结束
                        // {
                        //     //创建新的子进程
                            
                        //     createFork();
                        // }
                        // else //进程未结束
                        // {
                        //     pid = temp; 

                            //结束子进程
                            kill(pid, SIGTERM);
                            printf("子进程强制结束了\n");

                            //创建新的子进程
                            
                            createFork();
                        //}
                    }
                }
                break;
            
            case 25:
                //自动播放 or 取消自动播放
                if (menuFlag != 0)
                {
                    if (autoMusicFlag == 0)
                {
                    autoMusicFlag = 1;
                    if (menuFlag != 0 && musicFlag == 0)
                    {
                        menuFlag = 2;
                        lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                        musicTools(menuFlag);
                    }
                    else if (menuFlag != 0 && musicFlag == 1)
                    {
                        menuFlag = 4;
                        lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                        musicTools(menuFlag);
                    }
                }
                else
                {
                    autoMusicFlag = 0;
                    if (menuFlag != 0 && musicFlag == 0)
                    {
                        menuFlag = 1;
                        lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                        musicTools(menuFlag);
                    }
                    else if (menuFlag != 0 && musicFlag == 1)
                    {
                        menuFlag = 3;
                        lcdDrawBMP(0, 0, imagePaths[imageIndex], 0);
                        musicTools(menuFlag);
                    }
                }

                printf("自动播放 or 不自动播放");
                }
                break;

            default:
                printf("未知操作！\n");
                break;
            }
            //数据处理完成
            touchData = -1;
            touchFlag = 0;
        }
    }
    
}


