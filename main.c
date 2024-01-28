#include "commandhandlers.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>

char cmdline[128];
//int pid;
char *p;
FILE *f_cmd;
char tmp[256]="";
int main(int argc, char *argv[])
{
   int res;
   int len=128;
   struct sockaddr *cliaddr;
   char peer_name[256];

   tzset(); // for date/time requests
   // correct work dir 
   snprintf(tmp,256,"/proc/%d/cmdline",getpid());
   f_cmd = fopen(tmp,"r");
   if (f_cmd)
   {
    fgets(tmp,256,f_cmd);// path to "vmt_server"
    p=strrchr(tmp,'/');
    if (p)
     *p='\0'; // cut filename, only path remains
    if (chdir(tmp) == -1)
    {
     sprintf(msgbuf,"Cannot chdir to <%s> ! Terminated.",tmp);
     //writelog(msgbuf);
     syslog(LOG_INFO|LOG_LOCAL0,msgbuf);
     exit(-1);
    }
   } 
   else
   {
    sprintf(msgbuf,"Cannot open <%s> ! Terminated.",tmp);
//    writelog(msgbuf);
    syslog(LOG_INFO|LOG_LOCAL0,msgbuf);
    exit(-1);
   } 
   
   cliaddr=malloc(128);
   getpeername(0,cliaddr,&len);
   sprintf(peer_name,"Connection from: %s",sock_ntop(cliaddr,len));
   writelog(peer_name);
   free(cliaddr);   

   setvbuf( stdout, NULL, _IOLBF, 0 );
//   if (fgets ( cmdline, sizeof( cmdline ) , stdin ) != NULL)
   // get command from client and generate answer
   // until client disconnected
   while (fgets ( cmdline, sizeof( cmdline ) , stdin ) != NULL)
   {

         cmdline[strlen(cmdline)-2]='\0';
         if (!check_cmd_line(cmdline))
          do_command(cmdline, cmd_args);
         else
         {
          res=-1;
          write(0,&res,sizeof(res));
          return res;
         }
   }
  return 0;
}
