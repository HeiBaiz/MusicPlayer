#ifndef __MUSIC_H__
#define __MUSIC_H__

#include <sys/types.h>

/************************************************
 * 功能：音乐播放
 * 参数：
 *      *pid：进程号
 *         i：选择功能
 * 返回值：无
*************************************************/
int musicPlayer(pid_t *pid,int i);

#endif