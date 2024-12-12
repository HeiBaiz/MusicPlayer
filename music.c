#include "music.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

char path1[]="/usr/bin/madplay",path2[]="./1.mp3";
int j=1;
int musicFlag=0,temp;

#define musicMin 0
#define musicMax 3

int musicPlayer(pid_t *pid,int i){
    // pid_t pid = -2;
    switch(i){
        case 1:
            // 播放
            if(pid==-2){
                pid = fork();
                if (-1 == pid)
                {
                    perror("fork error");
                    return -1;
                } else if(pid==0) {
                    int res = execl(path1,"madpaly",path2,NULL);
                    if (res == -1)
                    {
                        perror("execl error");
                        return -1;
                    }
                }
            }
        case 2:
            // 上一曲
            kill(pid,SIGTERM);
            if(j-- == musicMin){
                j = musicMax;
            }
            sprintf(path2,"./%d.mp3",j);
            int res = execl(path1,"madpaly",path2,NULL);
            if (res == -1)
            {
                perror("execl error");
                return -1;
            }
        case 3:
            // 下一曲
            kill(pid,SIGTERM);
            if(j++ == musicMax){
                j = musicMin;
            }
            sprintf(path2,"./%d.mp3",j);
            int res = execl(path1,"madpaly",path2,NULL);
            if (res == -1)
            {
                perror("execl error");
                return -1;
            }
        case 4:
            // 暂停
            if(!musicFlag){
                kill(pid,SIGSTOP);
                musicFlag=1;
            }else {
                kill(pid,SIGCONT);
                musicFlag-0;
            }
            

    }
    execl()
}