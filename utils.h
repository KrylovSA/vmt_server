#ifndef UTILS_H
#define UTILS_H
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_GROUPS 5
#define MAX_LINE_LEN 256
#define WM_MONITOR 0
#define WM_DIAGNOSE 1

#define UNIX_DAY_DELTA 25569.0
#define SECS_PER_DAY 86400.0
#define UNIX_SEC_DELTA 2209161600 // delta between unix and win timestamps in seconds

#define LOCK_TMOUT 60

struct p_coll_data
{
// ServiceName:String; // service name (if vmt.exe compiled as a service)
    int adc_slot;   // ADC slot #
	int n_groups;   // quantity of groups
	int n_outputparams[MAX_GROUPS];// number of output parameters (OParams in VMT.INI) in groups
	int n_diagnoseparams[MAX_GROUPS];//  number of diagnostic parameters (DParams in VMT.INI) in groups
	int n_totalchannels[MAX_GROUPS];// number of channels in groups
	int n_channels[MAX_GROUPS];//   number of channels in groups (not include switched off channels)
	int n_samples[MAX_GROUPS];// number of samples in groups
	int n_diagnosesamples[MAX_GROUPS];// number of diagnostic samples in groups
	int b_ready;// True, when vmt.ini successfully processed
	int b_working;// is the collector working ?
	unsigned long hProcess;// handle of process ,used for checking status of the collector
	int is_stopping;// True, when collector is stopping ("cmd_stop" was sent, but collector does not answer yet)
	int n_calcparams; // Number of calculated parameters
} coll_data;
//int daytable[2][12]={31,28,31,30,31,30,31,31,30,31,30,31,31,29,31,30,31,30,31,31,30,31,30,31};

// buffer for creating log messages
char msgbuf[MAX_LINE_LEN];
FILE *F;

int writelog(char *s);
void crc_init();
unsigned long crc_cycle(unsigned long crc32, unsigned char * buf, size_t cnt);
char *sock_ntop(const struct sockaddr *sa, int salen);
int readvmtinifile(char *ininame, struct p_coll_data *p);
char *extractword(int n,char *s,char *delim);
char *getvaluebyname(char *line,char *name,FILE *F,int n_bytes_max);
int wordcount(char *s,char *delim);
void getvmtpath(char *buf,char *find_coll,char **vmtpath,char **collname);
void get_oper_list(char *operators,int len);
int processdatafile(char *filename,int workmode/*=WM_MONITOR*/);
int processtrend(char *path,char *Mask,time_t dtFrom,time_t dtTo,int workmode/* = WM_MONITOR*/);
time_t parsedata(char *s);
double unix_to_datetime(time_t t);
void  rewrite_time_stamp( int *changed);
void send_udp_note(char *buf_note,int buflen);
int makelock(char *lck);
int removelock(char *lck);
int clear_dir(char *path);
int unixfiletime_to_winfiletime(time_t fl_time);
void correct_case(char *str);
int move_file(char *src, char *dst);
#endif
