#ifndef SERV_H
#define SERV_H

#define SVC_RUNNING 1
#define SVC_STOPPED -1
#define SVC_NOTDEF 0

int waitforpidfile(int flag,char *serviceName,int TmOut);
int servicestart(char *ServiceName, int *errcode);
int servicestop(char *serviceName);
int servicegetstatus(char *serviceName);
#endif
