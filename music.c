#include "music.h"
#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

char path1[]="/usr/bin/madplay",path2[]="./1.mp3";
char url[15];
int flag_j=1;
int musicFlag=0,temp,res;
int status;

#define musicMin 1
#define musicMax 3

int musicPlayer(pid_t * pid,int i){
    // if(*pid == waitpid(*pid, &status, WNOHANG))
    switch(i){
    case 1:
        // 播放
        if(*pid==-2 && musicFlag==0){
            musicFlag = 1;
            *pid = fork();
            if (-1 == *pid) {
                perror("fork error");
                return -1;
            } else if(*pid==0) {
                sprintf(path2,"./%d.mp3",flag_j);
                int res = execl(path1,"madplay",path2,NULL);
                if (res == -1) {
                    perror("execl error");
                    return -1;
                }
            }
        } else if (musicFlag) {
            kill(*pid,SIGSTOP);
            musicFlag=0;
        } else {
            kill(*pid,SIGCONT);
            musicFlag=1;
        }
        break;
    case 2:
        // 上一曲
        if(musicFlag==0){
            kill(*pid,SIGCONT);
            musicFlag = 1;
        }
        kill(*pid,SIGTERM);
        if(flag_j-- == musicMin) {
            flag_j = musicMax;
        }
        sprintf(path2,"./%d.mp3",flag_j);
        
        *pid = fork();
        if(-1 == *pid) {
            perror("fork error");
            return -1;
        } else if(*pid==0) {
            res = execl(path1,"madplay",path2,NULL);
            if (res == -1)
            {
                perror("execl error");
                return -1;
            }
        }
        sprintf(url,"./%d.bmp",flag_j);
        lcdDrawBMP(url,0,0,0);
        break;
    case 3:
        // 下一曲
        if(musicFlag==0){
            kill(*pid,SIGCONT);
            musicFlag = 1;
        }
        kill(*pid,SIGTERM);
        if(flag_j++ == musicMax){
            flag_j = musicMin;
        }
        sprintf(path2,"./%d.mp3",flag_j);
        *pid = fork();
        if(-1 == *pid) {
            perror("fork error");
            return -1;
        } else if(*pid==0) {
            res = execl(path1,"madplay",path2,NULL);
            if (res == -1)
            {
                perror("execl error");
                return -1;
            }
        }
        sprintf(url,"./%d.bmp",flag_j);
        lcdDrawBMP(url,0,0,0);
        break;
    }
    printf("%s\n",path2);
}

