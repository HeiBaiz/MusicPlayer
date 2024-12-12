#ifndef __LCD_H__
#define __LCD_H__


#define COLOR_RED 0x00FF0000
#define COLOR_GREEN 0x0000FF00
#define COLOR_BLUE 0x000000FF
#define COLOR_WHITE 0x00FFFFFF
#define COLOR_BLACK 0
#define COLOR_ORANGE 0x00FFC125
#define COLOR_PURPLE 0x00D15FEE
#define COLOR_GRAY 0x00C1C1C1


/************************************************
 * 功能：初始化LCD屏幕
 * 参数：无
 * 返回值：
 *      -1：失败
 *       0：成功
*************************************************/
int lcdInit();

/************************************************
 * 功能：关闭LCD屏幕
 * 参数：无
 * 返回值：无
*************************************************/
void lcdClose();

/************************************************
 * 功能：刷背景色
 * 参数：
 *      color：要显示的背景颜色
 * 返回值：无
*************************************************/
void lcdBrushBG(unsigned int color);

/************************************************
 * 功能：在指定的位置画像素点
 * 参数：
 *      x,y：像素点的坐标
 *      color：像素点的颜色值
 * 返回值：无
*************************************************/
void lcdDrawPoint(int x,int y,unsigned int color);

/************************************************
 * 功能：在指定的位置画矩形(实心)
 * 参数：
 *      x0,y0：矩形显示的位置(左上顶点)
 *      w,h：矩形的宽度和高度
 *      color：矩形的填充颜色
 * 返回值：无
*************************************************/
void lcdDrawRect(int x0,int y0,int w,int h,unsigned int color);

/************************************************
 * 功能：在指定的位置画圆形(实心)
 * 参数：
 *      x0,y0：圆心的坐标
 *      r：半径
 *      color：圆形的填充颜色
 * 返回值：无
*************************************************/
void lcdDrawCircle(int x0,int y0,int r,unsigned int color);

/************************************************
 * 功能：在指定的位置显示图片
 * 参数：
 *      x0,y0：图片显示的位置(左上顶点的坐标)
 *      pic_path：要显示的图片的路径名
 *      kind：特效种类
 * 返回值：无
*************************************************/
void lcdDrawBMP(int x0,int y0,const char *pic_path, int kind);

/************************************************
 * 功能：loading动画的播放
 * 参数：
 *      x0,y0：图片显示的位置(左上顶点的坐标)
 *      num：图片个数
 *      kind：特效种类
 * 返回值：无
*************************************************/
void lcdBootDraw(int x0, int y0, int num, int kind);

/************************************************
 * 功能：播放启动画面，包括loading动画以及完成画面
 * 参数：无
 * 返回值：无
*************************************************/
void loading();

/************************************************
 * 功能：滑动界面锁屏，开机动画结束后出现需上滑的锁屏
 * 参数：无
 * 返回值：无
*************************************************/
void swipe();

/************************************************
 * 功能：锁屏界面
 * 参数：无
 * 返回值：无
*************************************************/
void lock();

/************************************************
 * 功能：打音乐播放器的工具栏
 * 参数：
 *      num：音乐播放栏打印的种类
 * 返回值：无
*************************************************/
void musicTools(int num);

/************************************************
 * 功能：电子音乐相册的主要实现函数
 * 参数：无
 * 返回值：无
*************************************************/
void musicPhotoAlbum();

/************************************************
 * 功能：创造一个子进程并执行execl函数去播放一首音乐
 *  并将子进程号返回给全局变量pid
 * 参数：无
 * 返回值：无
*************************************************/
void createFork();



#endif