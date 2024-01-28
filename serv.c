#include "serv.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

/*#define SVC_RUNNING  1
#define SVC_STOPPED  -1
#define SVC_NOTDEF  0
*/

#define WAIT_FOR_PID  10 // timeout for waiting *.pid file, sec

// wait for *.pid file of the service
int waitforpidfile(int flag,char *servicename,int tmout)
{
 time_t t;
 char pid_file[50];
 struct stat dumb; //only for checking existence

 t = time(NULL);
 sprintf(pid_file,"/var/run/%s.pid" ,servicename);
 while  (difftime(time(NULL),t) <= (double)tmout)
 {
  if (flag>0)
  {
   if (stat(pid_file,&dumb)!=-1)  // file exists
    return 0;
  }
  else
  {
   if (stat(pid_file,&dumb) == -1) // file not exists
    return 0;
  }
  sleep(1);
 }
 return -1; // wait failed
}


int servicestart(char *servicename, int *errcode)
{

 //int status;
 int vmt_pid, exit_code;
 //int res;

 errcode = 0;
 vmt_pid = fork();
 switch (vmt_pid)
 {
  case -1: return -1;
  case  0: {
            exit_code = execl(servicename,servicename,NULL);
            exit(exit_code);
           }
  default:
           {
            waitpid(vmt_pid,errcode,0);
            if (errcode == 0)
             return waitforpidfile(1,servicename,WAIT_FOR_PID);
            else
             return -1;
           }
 }
}

int servicestop(char *servicename)
{
 char pid_file[50];
 int pid;
 FILE *F;
 //int  err_code;
 int res=0;

 sprintf(pid_file,"/var/run/%s.pid", servicename) ;
 F=fopen(pid_file,"r");
 if (F)  // if pid file exists
 {
  pid_file[0]='\0';
  pid=-1;
  if (!fgets( pid_file,sizeof(pid_file),F ))
   writelog("servicestop error: cannot read *.pid file");
  fclose( F );
  if (pid_file[0])
  {
   pid=atoi(pid_file);
   if (pid > -1)
   {
    res=kill( pid, SIGTERM );
    if (res==0)
     res = waitforpidfile(0,servicename, WAIT_FOR_PID);
    return res;
   }
   else
   {
    writelog("servicestop error: error number in *.pid file");
    return -1;
   }
  }
  else
   return -1;
 }
 return res;
}

int servicegetstatus(char *servicename )
{
 char pid_file[50];
 int pid;
 FILE *F;
 int res = SVC_NOTDEF; // not defined status
 struct stat dumb;

 sprintf(pid_file,"/var/run/%s.pid", servicename);
 F=fopen(pid_file,"r");
 if (F)
 {
  pid=-1;
  pid_file[0]='\0';
  if (!fgets( pid_file,sizeof(pid_file),F))
   writelog("servicegetstatus error: cannot read *.pid file");
  fclose( F );
  if (pid_file[0])
  {
   pid=atoi(pid_file);
   if (pid > -1)
   {
//    sprintf(pid_file,"/proc/%d/cmdline",pid);
//    if (stat(pid_file,&dumb) != -1)
    if (kill(pid,0)!=-1)
     return SVC_RUNNING;
    else
     return SVC_STOPPED;
   }
   else
    writelog("servicegetstatus error: error number in *.pid file");
  }
 }
 else
  return  SVC_STOPPED; // stopped
 return res;
}
