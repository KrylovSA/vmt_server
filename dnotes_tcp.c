/*
 ------------------------------------------------------
 Module for sending TCP notices to client programs.
 Every time the file "cascad.sht" is updated, new notice is generated.
 ------------------------------------------------------
*/

#define _GNU_SOURCE	/* needed to get the defines */
#include "utils.h"
#include "commandhandlers.h"
#include "dnotes_tcp.h"
#include <fcntl.h>	/* in glibc 2.2 this has the needed
				   values defined */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/shm.h>



#define DN_MULTISHOT  0x80000000       /* Don't remove notifier*/
#define F_NOTIFY  1026 /*(1024+2)*/
#define F_SETSIG  10

void *p;
unsigned long bytes_to_send;
char *address;
int port;
int i,len,ret;
struct sigaction act;
//int fd_note;
//int note_active = 0;
char monitor_path[MAX_LINE_LEN]="";

// set notyfication on "Monitor" folder
void prepare_notification()
{

  FILE *f_pid;
 // int n_pid;
  char data[10]="abcdefghij";
  
	writelog("starting tcp_note_thread...");
	note_active=0;
 // ------ checking if monitoring folders are exist ----------------
	act.sa_sigaction = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGRTMIN + 1, &act, NULL);
/*
	snprintf(monitor_path,MAX_LINE_LEN-1,"%sMonitor",pre_path);
	fd_note = open(monitor_path, O_RDONLY);
	if (fd_note < 0)
	{
		writelog("tcp notification init error (open = -1)!");
		return;
	}
	fcntl(fd_note, F_SETSIG, SIGRTMIN + 1);
	fcntl(fd_note, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_MULTISHOT);
*/	
	note_active = 1;
	// --------------------------------------------------------------------
	// create "NN.pid" file
	// to recive UDP notyfications from VMT
	// NN - getpid
	snprintf(monitor_path,MAX_LINE_LEN - 1, "%s%d.pid",pre_path,getpid());
  f_pid = fopen(monitor_path ,"w+" );
  fwrite(data,sizeof(data),1,f_pid);
  fclose(f_pid);
  // --------------------------------------------------------------------	
	writelog("tcp notyfication started OK!");
}

// clear notyfication
void stop_notification()
{
//	close(fd_note);
	note_active=0;
	snprintf(monitor_path,MAX_LINE_LEN - 1, "%s%d.pid",pre_path,getpid());
	unlink(monitor_path); // remove "NN.pid" to stop notyfications
	writelog("TCP notyfication stopped.");
}
/*
//send notyfication to client
void handler(int sig, siginfo_t *sig_info, void *sig_context)
{

	F=tmpfile(); // we have to create temp file F, because "processdatafile" returns data there
	if (F)
	{
		snprintf(monitor_path,MAX_LINE_LEN-1,"%sMonitor/CASCAD.SHT",pre_path);
		if (!processdatafile(monitor_path,WM_MONITOR)) 
			sendnote();
		else
			writelog("tcp_note_handler: error processing CASCAD.SHT !");
		fclose(F);
	}
	else
	 writelog("tcp_note_handler: cannot create tempfile (tmpfile==NULL)!");
}
*/
void handler(int sig, siginfo_t *sig_info, void *sig_context)
{
	int hMapped;
	char *pMapped,*p;
	int res;
	char trend_fl_name[20]="\0";
	unsigned char len_flname;
	unsigned int buflen,buflen1;
	unsigned int *p_len;
	int b;
	char *sendbuf;
	unsigned long len,len1;
	int peer_len=128;
	struct sockaddr *cliaddr=NULL;
	struct timeval tm;
	fd_set rd;
	
	
	act.sa_sigaction = NULL; // switch handler off
	// get the name of the last trend file
	hMapped=shmget(VMT_NOTE_KEY + coll_data.adc_slot,VMT_NOTE_LEN,0);
	if (hMapped==-1)
	{
		res=-1;
		writelog("tcp_note handler: shmget error");
		note_active = 0;
		return ;
	}

	pMapped=shmat(hMapped,NULL,SHM_RDONLY);
	if (pMapped==(void*)-1)
	{
		res=-1;
		writelog("tcp_note handler: shmat error ");
		note_active = 0;
		return ;
	}
    // get buffer length from the end fo shared buffer 
	p_len = (unsigned int *)(pMapped + VMT_NOTE_LEN - sizeof(buflen));
	buflen = (unsigned int)(*p_len);
	buflen1 = htonl(buflen); // buffer length in network bytes order
	if (write(0, &buflen1, sizeof(buflen1))==-1) // sending length of stream (network bytes order)
	{   // if fail, set note_active=0 and main thread will terminate
		note_active = 0;
		return;
	}
	if (write(0, pMapped, buflen)==-1) // sending stream
	{
		note_active = 0;
		return;
	}
        
    // waiting for answer from client    
    res=0;
	while(res != -1)
	{
     FD_ZERO(&rd);
	 FD_SET(0,&rd);
	 tm.tv_sec=1;
	 tm.tv_usec=0;
	 res=select(0+1,&rd,NULL,NULL,&tm);
	 if (res > 0)
	 { // if answer received, finish handler
	  res=read(0, &b, sizeof(b));
	  if (res == -1)
	   break;
	  shmdt(pMapped);
	  act.sa_sigaction=handler;
	  return; // notyfication was sent correctly
	 } 
	}  
	// if we are here, we couldn't get answer
    // so stop notyfication 
	shmdt(pMapped);
	note_active = 0;

}
/*
// send UDP notice to clients in SendList
void sendnote()
{
	int b;
	int res;
	char *sendbuf;
	unsigned long len,len1;
	int peer_len=128;
	struct sockaddr *cliaddr=NULL;
	struct timeval tm;
	fd_set rd;
	

      writelog("sendnote begin!");
        act.sa_sigaction=NULL;
	len = ftell(F);
	sendbuf=calloc(len + 1,sizeof(char));
	rewind(F);
	fread(sendbuf,1,len,F);
	len1 = htonl(len);
	if (write(0, &len1, sizeof(len1))==-1) // sending length of stream (network bytes order)
	{   // if fail, set note_active=0 and main thread will terminate
		note_active = 0;
		free(sendbuf);
		return;
	}
	if (write(0, sendbuf, len)==-1) // sending stream
	{
		note_active = 0;
		free(sendbuf);
		return;
	}

//	if (getpeername(0,cliaddr,&peer_len)!=-1) // get ip of client
//		snprintf(msgbuf,MAX_LINE_LEN-1,"notification sent to: %s",sock_ntop(cliaddr,peer_len));
      writelog("note sent !");

      res=0;
	while(res != -1)
	{
         FD_ZERO(&rd);
	 FD_SET(0,&rd);
	 tm.tv_sec=1;
	 tm.tv_usec=0;
	 res=select(0+1,&rd,NULL,NULL,&tm);
	 if (res > 0)
	 {
	  res=read(0, &b, sizeof(b));
	  act.sa_sigaction=handler;
	  free(sendbuf);
	  return;
	 } 
	}  
	note_active = 0;
	free(sendbuf);
	return;
}
*/
