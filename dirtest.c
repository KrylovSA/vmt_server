#include "commandhandlers.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
   DIR *dir;
   struct dirent *p_dirent;
   
   printf("double=%d",sizeof(double));
   dir=opendir("/usr/local/Hercules/inetd");
   if (dir)
   {
    while(p_dirent=readdir(dir),p_dirent)
     printf(p_dirent->d_name,"\n\r");
    closedir(dir); 
   }
   else
    printf("opendir error!");
  return 0;
}
