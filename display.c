/* 循环显示几张图片，间隔时间1秒钟 */

#include "display.h"

int fb_size = 800*480*4;    //帧缓冲设备文件的大小

int fd_lcd; //帧缓冲设备文件的文件描述符
unsigned int *pmap; //映射区首地址

int arr1[800][480],abs_x[800],abs_y[480];

// 定义一个布尔数组来跟踪哪些元素已经被输出
int is_printed[800][480] = {0};

/************************************************
 * 功能：初始化LCD屏幕
 * 参数：无
 * 返回值：无
*************************************************/
int lcd_init() {
    // 打开帧缓冲设备文件(/dev/fb0)
    fd_lcd = open("/dev/fb0",O_RDWR);
    if (fd_lcd == -1){
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
void lcd_close() {
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
void lcd_brushBG(unsigned int color) {
    for (int y = 0; y < 480; y++){
        for (int x = 0; x < 800; x++){
            *(pmap+y*800+x) = color;
        }
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
void lcdDrawRect(int x0,int y0,int w,int h,unsigned int color) {
    for (int y = y0; y < y0+h; y++) {
        for (int x = x0; x < x0+w; x++) {
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
void lcdDrawCircle(int x0,int y0,int r,unsigned int color) {
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

/************************************************
 * 功能：在指定的位置画像素点
 * 参数：
 *      x,y：像素点的坐标
 *      color：像素点的颜色值
 * 返回值：无
*************************************************/
void lcdDrawPoint(int x,int y,unsigned int color) {
    if (x >= 0 && x < 800 && y >= 0 && y < 480){
        *(pmap+y*800+x) = color;
    }
}

/************************************************
 * 功能：显示图片
 * 参数：
 *      *pic_path：要显示的图片的路径
 *           flag：选择直接显示图片或存储
 *          x0,y0：图片显示位置
 * 返回值：无
*************************************************/
void lcdDrawBMP(const char *pic_path, int flag, int x0,int y0) {
    pthread_testcancel();  // 检查取消请求
    // 打开 bmp 图片
    int fd_bmp = open(pic_path,O_RDONLY);
    // printf("open %s\n",pic_path);
    if(fd_bmp == -1) {
        perror("open bmp error");
        return;
    }

    // 读取图片重要数据
    short ms,depth;     // 魔数、色深(2字节)
    int offset,w,h;     // 像素数组文件偏移量、宽度、高度(4字节)
    read(fd_bmp, &ms, 2);
    if(ms != 0x4D42){
        printf("this is not bmp!\n");
        close(fd_bmp);
        return;
    }
    // 读取数据
    lseek(fd_bmp,0x0A,SEEK_SET);
    read(fd_bmp,&offset,4);         // 像素数组文件偏移量
    lseek(fd_bmp,0x12,SEEK_SET);
    read(fd_bmp,&w,4);              // 宽度
    read(fd_bmp,&h,4);              // 高度
    lseek(fd_bmp,0x1C,SEEK_SET);
    read(fd_bmp,&depth,2);          // 色深
    // 打印图片的重要数据
    // printf("ms=0x%X,offset=0x%X,w=%d,h=%d,depth=%d\n",ms,offset,w,h,depth);

    // 读取像素数组数据(颜色值)
    int full_bytes = (4-(w * depth / 8)% 4)%4;     //每一行的填充字节数 = (4-一行像素点颜色值总字节数%4)%4
    int buf_size = (w*depth/8+full_bytes)*abs(h);    //像素数组大小 = (一行像素点颜色值的总字节数+填充字节数)*行数
    unsigned char color_buf[buf_size];  //像素数组数据
    lseek(fd_bmp,offset,SEEK_SET);
    read(fd_bmp,color_buf,buf_size);

    // 遍历整个图片，将每个像素点的颜色值取出来，按照argb的顺序重新排列
    unsigned char *p = color_buf;
    unsigned char a,r,g,b;
    for (int y = 0; y < abs(h); y++)
    {
        for (int x = 0; x < w; x++)
        {
            // 获取检查点状态
            pthread_testcancel();
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
            //将获取到颜色分量按照argb的顺序重新排列
            // a: 0000 0000
            // r: 1011 0100
            // g: 0001 0000
            // b: 1100 0101
            // a << 24
            //    0000 0000 0000 0000 0000 0000 0000 0000
            // r << 16
            //              1011 0100 0000 0000 0000 0000
            // g << 8
            //                        0001 0000 0000 0000
            // b
            //                                  1100 0101
            unsigned int color = a << 24 | r << 16 | g << 8 | b;
            //将像素点显示到屏幕上去
            if(h>0){
                if(flag){
                    arr1[x][h-1-y]=color;
                    is_printed[x][h-1-y] = 0;
                }else {
                    lcdDrawPoint(x+x0,h-1-y+y0,color);
                }
            }else{
                if(flag){
                    arr1[x][y]=color;
                    is_printed[x][y] = 0;
                }else {
                 }
            }
        }
        //每显示完一行像素点，就需要跳过后面的填充字节
        p += full_bytes;
    }
    
    //关闭图片文件
    close(fd_bmp);
}

/************************************************
 * 功能：将数组变为随机顺序数组
 * 参数：
 *      arr：要打乱顺序的数组
 *        n：要打乱的数组的长度
 * 返回值：无
*************************************************/
void randNums(int *arr, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = arr[i];
            arr[i] = arr[j];
            arr[j] = t;
        }
    }
}

/************************************************
 * 功能：生成自增数组
 * 参数：
 *      arr：要生成的数组
 *        n：要生成的数组的长度
 * 返回值：无
*************************************************/
void creNums(int *arr, size_t n) {
    for(int i=0;i<n;i++) {
        arr[i]=i;
    }
}

/************************************************
 * 功能：随即打印
 * 参数：
 *      arr：要打印的数组
 * 返回值：无
*************************************************/
void print_random_element(int array[ROWS][COLS],int x0,int y0) {
    int r, c;
    do {
        r = rand() % ROWS;
        c = rand() % COLS;
    } while (is_printed[r][c]); // 确保元素未被打印过
 
    is_printed[r][c] = 1; // 标记为已打印
    lcdDrawPoint(r+x0,c+y0,arr1[r][c]);
}

/************************************************
 * 功能：选择打印特效
 * 参数：
 *      x0,y0：打印坐标
 *       flag：选择特效
 * 返回值：无
*************************************************/
void randDisplay(int x0,int y0,int flag) {
    switch(flag) {
        case 1:
            printf("无特效\n");
            randDisplay0(x0,y0);
            printf("ok\n\n");
            break;
        case 2:
            printf("条纹\n");
            randDisplay1(x0,y0);
            printf("ok\n\n");
            break;
        case 3:
            printf("像素\n");
            randDisplay2(x0,y0);
            printf("ok\n\n");
            break;
        case 4:
            printf("网格\n");
            randDisplay3(x0,y0);
            printf("ok\n\n");
            break;
        case 5:
            printf("幕布\n");
            randDisplay4(x0,y0);
            printf("ok\n\n");
            break;
        case 6:
            printf("矩形\n");
            randDisplay5(x0,y0);
            printf("ok\n\n");
            break;
        default:
            printf("switch number-%d error!\n\n",flag);
            return;
    }
}

/************************************************
 * 功能：无特效打印图片
 * 参数：
 *      x0,y0：打印坐标
 * 返回值：无
*************************************************/
void randDisplay0(int x0,int y0) {
    // 显示图片
    for(int i=0;i<800;i++) {
        for(int j=0;j<480;j++) {
            pthread_testcancel();
            lcdDrawPoint(i+x0,j+y0,arr1[i][j]);
        }
    }
}

/************************************************
 * 功能：随机条纹打印图片
 * 参数：
 *      x0,y0：打印坐标
 * 返回值：无
*************************************************/
void randDisplay1(int x0,int y0) {
    // 随机条纹显示
    // 生成数组
    creNums(abs_x,800);

    // 打乱数组顺序，创建随机的显示顺序
    randNums(abs_x,800);

    // 显示图片
    for(int i=0;i<800;i++) {
        for(int j=0;j<480;j++) {
            pthread_testcancel();
            lcdDrawPoint(abs_x[i]+x0,j+y0,arr1[abs_x[i]][j]);
        }
        usleep(50);
    }
}

/************************************************
 * 功能：随机像素打印图片
 * 参数：
 *      x0,y0：打印坐标
 * 返回值：无
*************************************************/
void randDisplay2(int x0,int y0) {
    // 随机像素显示
    srand(time(0)); // 初始化随机数生成器
 
    // 输出二维数组的所有元素，确保每个元素只被输出一次
    for (int i = 0; i < 800 * 480; i++) {
        pthread_testcancel();
        if(i%800==0&&i!=0){
            usleep(1);
        }
        print_random_element(arr1,x0,y0);
    }
}

/************************************************
 * 功能：棋盘打印图片
 * 参数：
 *      x0,y0：打印坐标
 * 返回值：无
*************************************************/
void randDisplay3(int x0,int y0) {
    // 显示图片
    for(int k=0;k<2;k++){
        usleep(1000*100);
        for(int i=0;i<5;i++) {
            for(int j=0;j<5;j++) {
                for(int m=0;m<80;m++) {
                    for(int n=0;n<48;n++) {
                        pthread_testcancel();
                        lcdDrawPoint(x0+m+i*160+k*80,y0+n+j*96,arr1[m+i*160+k*80][n+j*96]);
                        lcdDrawPoint(x0+m+i*160+80-k*80,y0+n+j*96+48,arr1[m+i*160+80-k*80][n+j*96+48]);
                    }
                }
            }
        }
    }
}

/************************************************
 * 功能：幕布打印图片
 * 参数：
 *      x0,y0：打印坐标
 * 返回值：无
*************************************************/
void randDisplay4(int x0,int y0) {
    // 显示图片
    for(int i=0;i<400;i++){
        for(int j=0;j<480;j++){
            pthread_testcancel();
            lcdDrawPoint(x0+399-i,y0+j,arr1[399-i][j]);
            lcdDrawPoint(x0+400+i,y0+j,arr1[400+i][j]);
        }
        usleep(1000*3);  
    }
}

/************************************************
 * 功能：四格旋转打印图片
 * 参数：
 *      x0,y0：打印坐标
 * 返回值：无
*************************************************/
void randDisplay5(int x0,int y0) {
    int xMin,xMax,yMin,yMax;
    // 显示图片
    for(int m=0;m<80;m++){
        xMin=m*5,xMax=800-xMin;
        yMin=m*3,yMax=480-yMin;
        for(int i=0;i<800;i++){
            for(int j=0;j<480;j++){
                pthread_testcancel();
                if(i<xMin||i>xMax||j<yMin||j>yMax){
                    lcdDrawPoint(x0+i,y0+j,arr1[i][j]);
                }
            }
        }
        usleep(1000*3);
    }
}
