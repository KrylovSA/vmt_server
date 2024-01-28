#include "commandhandlers.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>

struct tm t;
time_t tt;
int main(int argc, char *argv[])
{
/*
  tt=time(NULL);
  localtime_r(&tt,&t);
  printf("%d.%d.%d %d:%d:%d is_dst=%d",t.tm_mday,t.tm_mon,t.tm_year,t.tm_hour,t.tm_min,t.tm_sec,t.tm_isdst);
  t.tm_mday=21;
  t.tm_mon=9;
  t.tm_year=2008-1900;
  t.tm_hour=12;
  t.tm_min=30;
  t.tm_sec=0;
  t.tm_isdst=-1;
  tt=mktime(&t);
  printf(ctime(&tt));
  */
  tzset();
  printf("Timezone=%d daylight=%d",timezone,daylight);
  return 0;
}
