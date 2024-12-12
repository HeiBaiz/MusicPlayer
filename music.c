#include "music.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

char path1[]="/usr/bin/madplay",path2[]="./.mp3";

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
                }
            }
        case 2:
            // 上一曲
            kill()
    }
    execl()
}