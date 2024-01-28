#include "utils.h"
#include "commandhandlers.h"
#include "zpipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <zlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <sys/timeb.h>

#define _GNU_SOURCE
//#define FIFO    "../log_buffer.tmp"
#define FIFO    "/usr/local/Hercules/inetd/log_buffer.tmp"


unsigned long crc_table[256];

//#define DEBUG

#ifndef DEBUG
int writelog(char *s)
{

// int fd;
	FILE *fd;
	int nwrite;
// char *p;
	int res;

  // open pipe

	res=0;
	fd=fopen(FIFO,"w"/*O_WRONLY*/);
	if (!fd)
	{
		syslog(LOG_INFO|LOG_LOCAL0,"WriteLog error: fopen = -1; %s",strerror(errno));
		return -1;
	}
  /* Sending message */
//  p = calloc(strlen(s) + 1, sizeof(char));
//  strcpy(p,s);
///  strcat(p,"\n");
	nwrite = fputs (s,fd);
	fputs("\r\n",fd);
	if (nwrite == -1)
	{
		syslog(LOG_INFO|LOG_LOCAL0,"WriteLog error: fputs= -1; %s",strerror(errno));
		res = -1;
	}
//  free(p);
//  fpclose(fd);
	fclose(fd);

	return res;

}
#else
int writelog(char *s)
{
	syslog(LOG_INFO||LOG_LOCAL0,s);
//	 printf("\n\r");
}
#endif


void crc_init()
{
	unsigned long t;
	int j,i;
	for( i=0;i<256;i++)
	{
		t=i;
		for(j=8;j>0;j--)
		{
			if(t&1)
				t=(t>>1)^0xedb88320;
			else
				t>>=1;
		}
		crc_table[i] = t;
	}
}

unsigned long crc_cycle(unsigned long crc32, unsigned char * buf, size_t cnt)
{
	size_t i;

	crc32=~crc32;
	for(i=0;i<cnt;i++)
	{
		crc32=crc_table[(crc32^buf[i])&0xff]^(crc32>>8);
	}
	crc32=~crc32;
	return crc32;
}

char *sock_ntop(const struct sockaddr * sa, int salen)
{
	char            portstr[7];
	static char     str[128];	/* Unix domain is largest */
	struct sockaddr_in *sin;

	switch (sa->sa_family)
	{
		case AF_INET:
			{
				sin = (struct sockaddr_in *) sa;
				if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
					return (NULL);
				if (ntohs(sin->sin_port) != 0)
				{
					snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
					strcat(str, portstr);
				}
				return (str);
			}
	}
	return NULL;
}
// parses VMT.INI file and fills CollData
int readvmtinifile(char *ininame, struct p_coll_data *p)
{

	char *s;
	char line[MAX_LINE_LEN+1];
	char tmpline[MAX_LINE_LEN+1];
	char chan[5];
 /*char iparams[256];
 char oparams[256];
 char dparams[256];  //  input , output and diagnostic parameters of group*/
	int i,j;//,nGr,nCh;  // # of group, # of channel
 //int last_lin;// the last line of group in VMT.INI
	int p2m,p2d; // number of samples, 2^ for monitoring and diagnostic modes
	FILE *F;
	int res;
 //int n_groups;
	long f_pos;

	res=-1;
	F=fopen(ininame,"r");
	if (F)
	{   
/*        // get ADC slot
        p->adc_slot = 0;
	    s=getvaluebyname(line,"ADC_Slot",F,MAX_LINE_LEN);
	    if (s)
         p->adc_slot = atoi(s);   
		else
		 writelog("Line 'ADC_Slot=' not found, set default (0)");
		 
		rewind(F);	*/
		p->n_groups=-1;
		s=getvaluebyname(line,"Params",F,MAX_LINE_LEN);
		if (s)
		{
			s=extractword(1,s," ");
			p->n_groups=-1;
			p->n_groups=atoi(s);

			if (p->n_groups==-1)
			{
				writelog("readvmtinifile: nGroups not found!");
				fclose(F);
				return -1;
			}
			/*
			else
			{
				sprintf(msgbuf,"readvmtinifile: found %d groups",p->n_groups);
				writelog(msgbuf);
			}
*/
		}
		else
		{
			writelog("Line 'Params=' not found !");
			fclose(F);
			return -1;
		}

  // get number of calculated parameters
		rewind(F);
		p->n_calcparams=0;
		s=getvaluebyname(line,"CParams",F,MAX_LINE_LEN);
		if (s)
		{
			p->n_calcparams=wordcount(s," ");
			/*
			sprintf(msgbuf,"readvmtinifile: found %d calculated parameters",p->n_calcparams);
			writelog(msgbuf);
			*/
		}
		else
			writelog("Calculated parameters not found.");

  // processing groups
		rewind(F);
		for (i=1;i<=p->n_groups;i++)
		{
//			sprintf(msgbuf,"---- Group #%d -----",i);
//			writelog(msgbuf);
			s=getvaluebyname(line,"IParams",F,MAX_LINE_LEN);
			if (s)
			{
				strncpy(tmpline,s,MAX_LINE_LEN); //save line to temp buf, becose extractword will break the line
				s=extractword(1,s," ");
				if (s)
				{
					p->n_channels[i-1]=atoi(s); // number of channels
//					sprintf(msgbuf,"Number of channels = %d",p->n_channels[i-1]);
//					writelog(msgbuf);
				}
				strcpy(line,tmpline);
				s=extractword(4,line," ");
				if (s)
				{
					p2m=atoi(s);
					p->n_samples[i-1]=pow(2,p2m); // number fo monitor samples
//					sprintf(msgbuf,"monitor samples = %d",p->n_samples[i-1]);
//					writelog(msgbuf);
				}
				strcpy(line,tmpline);
				s=extractword(5,line," ");
				if (s)
				{
					p2d=atoi(s);
					p->n_diagnosesamples[i-1]=pow(2,p2d); // number of diagnose samples
//					sprintf(msgbuf,"diagnose samples = %d",p->n_diagnosesamples[i-1]);
//					writelog(msgbuf);
				}
			}
			else
			{
				writelog("IParams not found!!");
				fclose(F);
				return -1;
			}

			p->n_totalchannels[i-1]=p->n_channels[i-1];
			f_pos=ftell(F);
			for (j=1;j<=p->n_totalchannels[i-1];j++)
			{
				sprintf(chan,"Ch%d",j);
				s=getvaluebyname(line,chan,F,MAX_LINE_LEN);
				if (s)
				{
					s=extractword(3,s," ");
					if (atoi(s)==0)
						p->n_channels[i-1]--;
				}
			}
//			if (p->n_channels[i-1] != p->n_totalchannels[i-1])
//				writelog("Some channels are off!");

			fseek(F,f_pos,SEEK_SET);
   // output parameters of the group
			s=getvaluebyname(line,"OParams",F,MAX_LINE_LEN);
			if (s)
			{
				p->n_outputparams[i-1]=wordcount(s," ");
//				sprintf(msgbuf,"monitor parameters = %d",p->n_outputparams[i-1]);
//				writelog(msgbuf);
			}
			else
				writelog("OParams not found !!");
   // diagnostic parameters in group
			fseek(F,f_pos,SEEK_SET);
			s=getvaluebyname(line,"DParams",F,MAX_LINE_LEN);
			if (s)
			{
				p->n_diagnoseparams[i-1]=wordcount(s," ");
//				sprintf(msgbuf,"diagnose parameters = %d",p->n_diagnoseparams[i-1]);
//				writelog(msgbuf);
			}
			else
				p->n_diagnoseparams[i-1]=0;
			fseek(F,f_pos,SEEK_SET);	
   // this group is done,go next
//			sprintf(msgbuf,"------- end group --------");
//			writelog(msgbuf);
		}// for j:=1 to Groups do
	}
	else
	{
		writelog("Cannot open 'VMT.INI'");
		fclose(F);
		return -1;
	}
//	writelog("readvmtini done.");
	fclose(F);
	return 0;

}
char *extractword(int n,char *s,char *delim)
{
	char *p;
	int i;

	if (!s)
		return NULL;
	p=strtok(s,delim);
	for (i=2;i<=n;i++)
	{
		p=strtok(NULL,delim);
		if (!p)
			break;
	}
	return p;
}
int wordcount(char *s,char *delim)
{
	char *p;
	int i;

	if (!s)
		return 0;
	i=0;
	p=strtok(s,delim);
	while (p)
	{
		i++;
		p=strtok(NULL,delim);
	}
	return i;
}
char *getvaluebyname(char *line,char *name,FILE *F,int n_bytes_max)
{

	if (!F)
		return NULL;
	while (fgets(line,n_bytes_max,F))
	{
		if (strstr(line,name/*,strlen(name)*/))
			return extractword(2,line,"=");
	}
	return NULL;
}
// get collector's name and path from vmt_note.ini
/*
 buf - buffer for reading vmt_note.ini
 operators - buffer for operators list
 len - length of "operators" buffer
*/
void get_oper_list(char *operators,int len)
{
	FILE *F;
	char *p;
	int total_len=0;
	char *buf; // buffer for reading vmt_note.ini

	*operators='\0';
	F=fopen("../vmt_note.ini","r");
	if (F)
	{
		buf=calloc(MAX_LINE_LEN,sizeof(char));  
		while(fgets(buf,MAX_LINE_LEN-1,F))
		{
			if (strcasestr(buf,"[Clients]"))
			{
				while(!feof(F))
				{ 
					fgets(buf,MAX_LINE_LEN-1,F);
					if (strcasestr(buf,"Operator"))
					{ 
						p=strtok(buf,"=");
						p=strtok(NULL,"=");
						if (p)
						{
							*(p+strlen(p)-1)='\0'; // remove ENTER
							total_len += strlen(p) + 1; // check length of buffer
							if (total_len < len)
							{
								strcat(operators," ");  
								strcat(operators,p);
							}
							else
							{ // buffer is full
								fclose(F);   
								free(buf);
								return; 
							} 
						}
					}
					else
					{ // end of <OperatorN=> lines
						fclose(F); 
						free(buf);  
						return; 
					} 
				}
			}
		}
		free(buf);
		fclose(F);
	}
	else
		writelog("Cannot open <vmt_note.ini>");
}
// get collector's name and path from vmt_note.ini
void getvmtpath(char *buf,char *find_coll,char **vmtpath,char **collname)
{
	FILE *F;

	*buf='\0';
	*collname=NULL;
	*vmtpath=NULL;
	coll_data.adc_slot = 0;
	F=fopen("../vmt_note.ini","r");
	if (F)
	{
		while(fgets(buf,MAX_LINE_LEN,F))
		{
			if (strcasestr(buf,"[Collectors]"))
			{
        if (!find_coll || !strcasecmp(find_coll,"NULL")) // if collector's name not set
        { 
				 fgets(buf,MAX_LINE_LEN,F);
				 *collname=strtok(buf,"=");
				 *vmtpath=strtok(NULL,"=");
				 *(*vmtpath+strlen(*vmtpath)-1)='\0'; // remove ENTER
				 if (*(*vmtpath+strlen(*vmtpath)-1)!='/') // add final slash, if not present
					strcat(*vmtpath,"/");
				 fclose(F);
				 sprintf(msgbuf,"getvmtpath: found collector %s; path <%s>",*collname,*vmtpath);
				 writelog(msgbuf);
				 return;
        }
        else
        { // searching for given collector line
         while (fgets(buf,MAX_LINE_LEN,F))
         {
          if (*buf=='[') // new section begins, exit
           return;
          *collname=strtok(buf,"=");
          if (!strcmp(*collname,find_coll)) // found given collector     
          {
				   *vmtpath=strtok(NULL,"=");
				   *(*vmtpath+strlen(*vmtpath)-1)='\0'; // remove ENTER
				   if (*(*vmtpath+strlen(*vmtpath)-1)!='/') // add final slash, if not present
					  strcat(*vmtpath,"/");
				   fclose(F);
				   sprintf(msgbuf,"getvmtpath: found collector %s; path <%s>",*collname,*vmtpath);
				   writelog(msgbuf);
				   return;
          }
          coll_data.adc_slot++;
         }   
        }
         
			}
		}
		fclose(F);
		writelog("Section [Collectors] not found");
	}
	else
		writelog("Cannot open <vmt_note.ini>");
}
// parses trend (*.sh?) file and writes to ToStream parameters values (for UDP notices)
int processdatafile(char *filename,int workmode/*=WM_MONITOR*/)
{
	char *outbuf;     // buffer for packed file
// OutBytes:Integer;   // number of bytes in packed file
	FILE *src;
	FILE *tmp;
	double mtime;    // date/time of the file
	double rtime;    // date/time of the file
 // -------- collector's settings -----------------
// groups:WORD; // number of groups
// ChanDataLen:Integer;// total size of channel output data (e.g. <Number_Of_Parameters>*SizeOf(Double))
	int nsample, chandatalen, channels;// number of samples
 // ---------------------------------------------
	int k,i,j;              // auxiliary vars
// nAggr:Integer;              // number of the collector in AggrNames
	char trendfilename[10];   // name of the trend file
	unsigned char len_fl_name;

	// Calibr:Double;              // auxiliary vars
	double levels;              // auxiliary vars
	char *p_last_delim;

 // -------------------
// sz:DWORD;                   // size of file
// NumberOfBytesRead:DWORD;    // number of bytes read from file
// hFile:THandle;              // handle of file
 // -------------------

// tmp:TMemoryStream;        // auxiliary vars

// fInfo:_Stat;

// ------------- read file into memory buffer ------------------------------------
	src=fopen(filename,"r");
	if (!src)
	{
		sprintf(msgbuf,"processdatafile: cannot open %s; errno=%d",filename,errno);
		writelog(msgbuf);
		return -1;
	}

	if (strcasestr(filename,"CASCAD.SHT"))
	{
		outbuf=calloc(1024,sizeof(char));
		while(!feof(src)&&!ferror(src))
		{
			fread(outbuf,1,1024,src);
			fwrite(outbuf,1,1024,F);
		} 
		fclose(src);
		free(outbuf); 
		return 0;
	}

	tmp=tmpfile(); // creating temporary file for unpacking
 //  tmp=fopen("tmp.sht","w+");
	if (!tmp)
	{
		writelog("processdatafile: cannot create temp file !");
		fclose(src);
		return -1;
	}

	if (inf(src,tmp) != Z_OK)
	{
		writelog("processdatafile: unpacking error!");
		fclose(src);
		fclose(tmp);
		return -1;
	}

	fflush(tmp);   
	rewind(tmp);   
	fseek(tmp,15+1,SEEK_CUR); // skip machine name String[15]
/*
tmp.Read(Aggrname,SizeOf(AggrName)); // read the name of the collector
//
nAggr:=-1;
for k:=0 to AggrNames.Count-1 do
 if AggrNames[k]=UpperCase(AggrName) then
 begin
  nAggr:=k;
  break;
 end;
if nAggr=-1 then  raise Exception.Create(msg_IncorrectColl); // incorrect collector's name
*/
//groups:=coll_data.n_groups;

	fread(&mtime, 1,sizeof(mtime),tmp); // time of trend file
	fwrite(&mtime,1,sizeof(mtime),F); // write to output stream
//fread(&len_fl_name, 1,sizeof(len_fl_name),tmp); // length of trend file name
	fseek(tmp,sizeof(len_fl_name),SEEK_CUR);
//fwrite(&len_fl_name,1,sizeof(len_fl_name),F); // write to output stream
	fseek(tmp,sizeof(trendfilename),SEEK_CUR); // trend file name
	p_last_delim=strrchr(filename,'/');
	if (!p_last_delim)
		p_last_delim=filename;
	else
		p_last_delim++; 
	strncpy(trendfilename,p_last_delim,sizeof(trendfilename));
//for (i=0;i<sizeof(trendfilename);i++)
	i=0;
	while (trendfilename[i])
	{
		trendfilename[i]=toupper(trendfilename[i]); // make upper case
		i++;
	} 
	len_fl_name=strlen(trendfilename);
	fwrite(&len_fl_name,1,sizeof(len_fl_name),F); // write to output stream
	fwrite(trendfilename,1,sizeof(trendfilename),F); // write to output stream
	for (k=0;k<coll_data.n_groups;k++)
	{
		chandatalen=0;
		nsample=0;
		switch (workmode) 
		{
			case WM_MONITOR:
				{
					chandatalen = coll_data.n_outputparams[k]; // number of output parameters in group (OParams in VMT.INI)
					nsample = coll_data.n_samples[k];// number of samples in group
					break;
				}
			case WM_DIAGNOSE:
				{
					chandatalen = coll_data.n_diagnoseparams[k]; // number of diagnostic parameters in group (DParams in VMT.INI)
					nsample = coll_data.n_diagnosesamples[k];// number of diagnostic samples in group
					break;
				}
		} // switch 
		channels = coll_data.n_channels[k];// number of channels in group

		fread(&rtime,1,sizeof(rtime),tmp);
		fwrite(&rtime,1,sizeof(rtime),F);    // RPM

		fseek(tmp,4 + 8,SEEK_CUR); // skip number of samples (DWORD) and time step (Double)
		for (i=1;i<=channels;i++)
		{
			fseek(tmp,8,SEEK_CUR);      // channel's calibration coefficient (Double)
			for (j=1;j<=chandatalen;j++)
			{
				fread(&levels,1,sizeof(levels),tmp); // read parameter value from source file
				fwrite(&levels,1,sizeof(levels),F);  // write to output stream
			}
			fseek(tmp,nsample * 2,SEEK_CUR);  // skip realize data
		} // for i:=1 to Chanls do
	} // for k:=1 to groups do
// if calculated parameters are present
	if ((workmode == WM_DIAGNOSE) && (coll_data.n_calcparams > 0)) 
	{
		for (j=1;j <= coll_data.n_calcparams;j++)
		{
			fread(&levels,1,sizeof(levels),tmp); // read parameter value from source file
			fwrite(&levels,1,sizeof(levels),F);  // write to output stream
		}
	}
// all done, OK
	fclose(src);
	fclose(tmp);
//sprintf(msgbuf,"File %s procssed OK!",filename);
//writelog(msgbuf);
	return 0;
}

// =======================================================================================
// get trend files, uses path and mask, between dtFrom and dtTo
// result will be stored in FILE *F
int processtrend(char *path,char *mask,time_t dt_from,time_t dt_to,int workmode /*= wm_Monitor*/)
{
// dt:TDateTime;      // stores TDatetime of file
	int n_files_found=0;   // number of files found
	DIR *dir;
	char *fl_name;//[MAX_LINE_LEN];
	struct dirent *dir_entry;
	struct stat buf_st;

	writelog("----- ProcessTrend  ----");
// sprintf(msgbuf,"dt_from=%d dt_to=%d",dt_from, dt_to);
// writelog(msgbuf);
 // start searching files, according given mask
	dir = opendir(path);
	if (dir)
	{
		fl_name=calloc(MAX_LINE_LEN+1, sizeof(char));
		while (dir_entry=readdir(dir),dir_entry)
		{  
			snprintf(fl_name,MAX_LINE_LEN,"%s/%s",path,dir_entry->d_name); // full path to file
//    writelog(fl_name);
			if (strcasestr(dir_entry->d_name , mask)) // it seems to be "our" file
//    if (!fnmatch(mask, dir_entry->d_name, FNM_CASEFOLD) // compare filename with mask
			{
				if (stat(fl_name,&buf_st) == 0)
				{
//       sprintf(msgbuf,"st_mtime=%d st_atime=%d st_ctime=%d",buf_st.st_mtime,buf_st.st_atime,buf_st.st_ctime);
//       writelog(msgbuf);
					if ((buf_st.st_mtime >= dt_from) && (buf_st.st_mtime <= dt_to)) // check last access time
					{
						if (processdatafile(fl_name,workmode))
						{
							sprintf(msgbuf,"File <%s> error!",dir_entry->d_name);   
							writelog(msgbuf);
						} 
						else
							n_files_found++;
					}
				}
				else
				{
					sprintf(msgbuf,"processtrend stat error: file <%s> !",dir_entry->d_name);      
					writelog(msgbuf);
				}
			}     
		} // while (dir_entry=readdir(dir),dir_entry)
		sprintf(msgbuf,"Found %d files.",n_files_found);
		writelog(msgbuf);
		writelog("---------------------------------");
		closedir(dir);
		free(fl_name);
		return 0;
	}
	else
	{
		sprintf(msgbuf,"Cannot open directory <%s> !!",path);
		writelog(msgbuf);     
		writelog("---------------------------------");
		return -1;
	}
}

// convert string like "10.10.2004 11:00:00" to TDateTime type
// Differs from "StrToDateTime" that does not use regional settings
time_t parsedata(char *s)
{
// unsigned int a_year, a_month, a_day, a_hour, a_minute, a_second, a_milli_second;
	char *s_date;
	char *s_time;
	struct tm dtm;

	s_date = strtok(s," ");    // date part
	s_time = strtok(NULL," "); // time part
	if (s_date)
	{
		dtm.tm_mday = atoi(strtok(s_date,"."));
		dtm.tm_mon = atoi(strtok(NULL,"."))-1;
		dtm.tm_year = atoi(strtok(NULL,".")) - 1900;
	}
	else
		return -1;
	if (s_time)
	{ 
		dtm.tm_hour = atoi(strtok(s_time,":"));
		dtm.tm_min = atoi(strtok(NULL,":"));
		dtm.tm_sec = atoi(strtok(NULL,":"));
	}
	else
		return -1; 
	dtm.tm_isdst = -1;//daylight; // correction !
// sprintf(msgbuf,"%d.%d.%d %d:%d:%d",dtm.tm_mday,dtm.tm_mon,dtm.tm_year,dtm.tm_hour,dtm.tm_min,dtm.tm_sec);
// writelog(msgbuf);
	return  mktime(&dtm);
}
double unix_to_datetime(time_t t)
{
     struct timeb tmb;
     unsigned long tsec;
//   struct timeval tv;
//   gettimeofday(&tv,NULL);
   
        ftime(&tmb);
	tsec = tmb.time - (tmb.timezone * 60); // timezone corretion (tmb.time is UTC)
	tsec += (tmb.dstflag  * 3600); // daylight correction
//	sprintf(msgbuf,"timezone=%d daylight=%d",tmb.timezone,tmb.dstflag);
//	writelog(msgbuf);
	return (tsec/SECS_PER_DAY + UNIX_DAY_DELTA);	
}
// check [TimeStamp] section in VMT.INI
// and correct it if needed
void  rewrite_time_stamp( int *changed)
{
	struct stat st;
	time_t fl_age_ini;
	char *buf=NULL;
	char *vmt_path=NULL;
	char *p;
	FILE *f_vmt;
	int n_pos;

	*changed = -1;

	vmt_path=calloc(MAX_LINE_LEN,sizeof(char));
	snprintf(vmt_path, MAX_LINE_LEN-1,"%svmt.ini",pre_path);
	if (stat(vmt_path,&st) == -1)
	{
		snprintf(msgbuf,MAX_LINE_LEN-1,"rewrite_time_stamp: stat=-1 on file <%s>",vmt_path);     
		writelog(msgbuf);
		free(vmt_path);
		return;  
	}
	f_vmt = fopen(vmt_path,"r");
	if (!f_vmt)
	{
		snprintf(msgbuf,MAX_LINE_LEN-1,"rewrite_time_stamp: fopen=-1 on file <%s>",vmt_path);       
		writelog(msgbuf);
		free(vmt_path);
		return;
	}
	buf=calloc(MAX_LINE_LEN,sizeof(char));
   // search [TimeStamp] section
	while(fgets(buf,MAX_LINE_LEN,f_vmt))
	{
		if (strcasestr(buf,"[TimeStamp]"))
		{
			n_pos=ftell(f_vmt);  // save position in vmt.ini
			fgets(buf,MAX_LINE_LEN,f_vmt); // take next line
			if (strcasestr(buf,"Time")) // must be Time=NNNNNNN
			{ 
				p=strtok(buf,"=");
				p=strtok(NULL,"="); // skip to time
				if (p)
				{
					*(p+strlen(p)-1)='\0'; // remove ENTER
					fl_age_ini = atoi(p);  // timestamp in vmt.ini
					if ((st.st_mtime - fl_age_ini) > 5)
					{
						*changed=1;  // timestamps not equal, so file was changed
						fclose(f_vmt);
            // now cat "Time=" and then add new line with timestamp
						truncate(vmt_path, n_pos); // truncate file till "Time=" string 
						f_vmt=fopen(vmt_path,"r+");
						fseek(f_vmt, 0, SEEK_END); // re-open vmt.ini and set cursor to EOF
						stat(vmt_path,&st);
						snprintf(buf,MAX_LINE_LEN-1,"Time=%d\n",(unsigned int)(st.st_mtime));
						fputs(buf,f_vmt); // write new Time=....
						fclose(f_vmt);
						free(vmt_path);
						free(buf);
						return;
					}
					else
					{
						*changed=0; // timestamps are equal, file was not changed
						fclose(f_vmt);
						free(vmt_path);
						free(buf);
						return;
					}
				}
			}
		}
	} // while(fgets(buf,MAX_LINE_LEN,f_vmt))
    // line "[TimeStamp]" not found in VMT.INI
	*changed=1;
	fclose(f_vmt);
        f_vmt=fopen(vmt_path,"r+");
	
	fseek(f_vmt, 0, SEEK_END); // re-open vmt.ini and set cursor to EOF
	stat(vmt_path,&st);
        fputs("[TimeStamp]\n",f_vmt);
	snprintf(buf,MAX_LINE_LEN-1,"Time=%d\n",(unsigned int)st.st_mtime);
	fputs(buf,f_vmt);// write new Time=....
	fclose(f_vmt);
	free(vmt_path);
	free(buf);
	return;
}        
// clearing directory <path>             
int clear_dir(char *path)
{
	struct dirent *dir_entry ;
	DIR *dir;
	char *fl_name;
	int failed=0;

	dir = opendir(path);
	if (dir)
	{
		fl_name=calloc(MAX_LINE_LEN, sizeof(char));
		snprintf(msgbuf,MAX_LINE_LEN-1,"clear_dir: start clearing <%s>...",path);
		writelog(msgbuf);  
		while (dir_entry=readdir(dir),dir_entry)
		{  
		     if (strcmp(dir_entry->d_name,".") && strcmp(dir_entry->d_name,".."))
		     {
			strcpy(fl_name,path); // create path to file
			if (*(fl_name+strlen(fl_name)-1)!='/') // add last / if needed
				strcat(fl_name,"/");    
			strcat(fl_name,dir_entry->d_name); // full path to file
			if (unlink(fl_name) == -1) 
			{ // error deleting file
				snprintf(msgbuf,MAX_LINE_LEN-1," cannot delete <%s>",dir_entry->d_name);
				writelog(msgbuf);     
				failed++;
			}
		      }	
		}
		free(fl_name);
		closedir(dir);
		if (failed)
		{
			writelog(" some files was not deleted !");     
			writelog("clear_dir: end work.");     
			return -1; 
		} 
		else 
		{
			writelog(" cleared OK!");             
			writelog("clear_dir: end work.");     
			return 0;
		} 
	} // cannot open <path>
	else
	{
		snprintf(msgbuf,MAX_LINE_LEN-1,"clear_dir: opendir=-1");
		writelog(msgbuf);     
		return -1;
	}    
}
void send_udp_note(char *buf_note,int buflen)
{
	char *oper_list;
	char *p, *p_port;
	char ip_port[30]=""; // for address string, e.g. "192.168.1.10:8888"
	int port;
	struct sockaddr_in sendaddr;
	int sender; // socket for sending UDP notyfication

	sender=socket(PF_INET,SOCK_DGRAM,0);
	if (sender == -1)
	{
		writelog("send_udp_note: error creating socket!");
		return;
	} 
	oper_list = calloc(MAX_LINE_LEN,sizeof(char));   
	get_oper_list(oper_list, MAX_LINE_LEN);// getting list of operators
	if (*oper_list) // if present at list one...
	{
		p=oper_list;
		while(p)
		{
			p=strtok(p, " "); // set pointer to next address (space delimited)
			strncpy(ip_port,p,sizeof(ip_port)-1);
			p_port=strchr(ip_port,':'); // find port
			port=-1;
			if (p_port)
			{
				*p_port='\0';
				p_port++; 
				port=atoi(p_port);
			}
			if (port==-1)
				port=8888; // if port not found, set default 8888
       // Now send UDP notyfication to operator
			sendaddr.sin_family = AF_INET;
			sendaddr.sin_addr.s_addr = inet_addr(ip_port);
			sendaddr.sin_port = htons(port);
       // sending buf_note to operator
			if (sendto(sender, buf_note, buflen, 0, (struct sockaddr *)&sendaddr, sizeof(sendaddr)) == -1)
			{
				snprintf(msgbuf,MAX_LINE_LEN-1,"send_udp_note: error sending to %s:%d; errno=%d",ip_port,port,errno);
				writelog(msgbuf);
			}  
			else
			{
				snprintf(msgbuf,MAX_LINE_LEN-1,"send_udp_note: notyfication sent to %s:%d",ip_port,port);
				writelog(msgbuf);
			}
       // move to next address in oper_list
			p=strtok(NULL," ");       
		}
	}  
  // freeing and closing
	free(oper_list); 
	close(sender);
}
/* make block on lck file*/
int makelock(char *lck)
{
	int fd;
	time_t t;

	t = time(NULL);
	fd=-1;
	while(((time(NULL)-t) <= LOCK_TMOUT) && (fd < 0))
		fd=open(lck,O_RDWR|O_CREAT|O_EXCL,384);
	if (fd < 0) // could not do lock
	{
        writelog("Could not create lock !!");
		close(fd);
		return -1;
	}
	else
	{   // lock file created, exit
        writelog("Lock created OK!!");
		close(fd);
		return 0;
	}	
}
// remove lock on lckfile
int removelock(char *lck)
{
  int b;
  b=unlink(lck);
  if (b != -1)
   writelog("Lock removed!");
  else
   writelog("Cannot remove lock!!"); 
  return b;
}	
// convert unix timestamp to windows timestamp
int unixfiletime_to_winfiletime(time_t fltime)
{
 unsigned short year,month,day,hour,min,sec;
 struct tm tmm;
 unsigned short hi_word,lo_word;
 
 localtime_r(&fltime, &tmm);
 year=tmm.tm_year + 1900;
 month=tmm.tm_mon + 1;
 day=tmm.tm_mday;
 hour=tmm.tm_hour;
 min=tmm.tm_min;
 sec=tmm.tm_sec;
 
 lo_word = (sec>>1)|(min<<5)|(hour<<11);
 hi_word = day|(month<<5)|((year-1980)<<9); 
 return lo_word|(hi_word<<16);
}
void correct_case(char *str)
{
    char *p;
	//change '\' to '/'	
	while (p=strchr(str, '\\'),p)
		*p='/';
	// correct case of filename		if (p=(char*)strcasestr(str,"trends"),p)
    if (p=(char*)strcasestr(str,"trends"),p)
		memcpy(p,"Trends",6);
	if (p=(char*)strcasestr(str,"minuts"),p)
		memcpy(p,"Minuts",6);
	if (p=(char*)strcasestr(str,"hours"),p)
		memcpy(p,"Hours",5);
	if (p=(char*)strcasestr(str,"rpms"),p)
		memcpy(p,"Rpms",4);
	if (p=(char*)strcasestr(str,"bands"),p)
		memcpy(p,"Bands",5);
	if (p=(char*)strcasestr(str,"dlevels"),p)
		memcpy(p,"dlevels",7);
	if (p=(char*)strcasestr(str,"appr"),p)
		memcpy(p,"Appr",4);
	if (p=(char*)strcasestr(str,"matrix"),p)
		memcpy(p,"matrix",6);
	if (p=(char*)strcasestr(str,"levels"),p)
		memcpy(p,"levels",6);
	if (p=(char*)strcasestr(str,"vmt"),p)
		memcpy(p,"vmt",3);
	if (p=(char*)strcasestr(str,"mathuserfunc.dll"),p)
		memcpy(p,"MathUserFunc.dll",16);
    
}
// copying file from src to dst
int move_file(char *src, char *dst)
{
    FILE *fsrc, *fdst;
    char *tmpname = NULL;
    char *buf;
    char *p;
    int bytes_read;
    struct stat st;

    if (stat(dst,&st) != -1) // if "dst" already exists
    {
    // rename dst to *.bak name
    tmpname = calloc(MAX_LINE_LEN,sizeof(char));
    strcpy(tmpname,dst);
    strcat(tmpname,".bak");
    unlink(tmpname); // delete previous *.bak file if exists
        
    if (rename(dst,tmpname)) // rename to *.bak
    {
     writelog("move_file: cannot rename to *.bak !!") ;
     free(tmpname);
		 return -1;    
    }
    }
    buf = calloc(1024,sizeof(char));
    fsrc = fopen(src,"r");
    if (fsrc)
    {
       fdst=fopen(dst,"w");
       if (fdst)
       {  // start copying
          while (!feof(fsrc))
          {
           bytes_read = fread(buf,1,1023,fsrc);
           fwrite(buf,1,bytes_read,fdst);
          } 
          fclose(fsrc);
          fclose(fdst);
          if (tmpname)
           free(tmpname);
          free(buf);
          return 0;
       }
       else
       {
        writelog("move_file: cannot open destination file !!") ;           
        fclose(fsrc);
        free(buf);
        // if error and exists *.bak file , rename it back to dst
        if (tmpname)
        {
         rename(tmpname,dst);
         free(tmpname);
        } 
        return -1;
       }
       
    }
    else
    {
      writelog("move_file: cannot open source file !!") ;           
      free(buf);
      // if error and exists *.bak file , rename it back to dst
      if (tmpname)
      {
       rename(tmpname,dst);
       free(tmpname);
      } 
      return -1;
    }
}   
