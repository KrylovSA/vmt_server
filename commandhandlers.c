#include "commandhandlers.h"
#include "utils.h"
#include "serv.h"
#include "zpipe.h"
#include "dnotes_tcp.h"
//#include "dscud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <zlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>

#define MAX_PATH 256

#define GET_COLL_SETTINGS "get_coll_settings"
#define GET_NOTE_SETTINGS "get_note_settings"
#define GET_LAST_FILE "get_last_file"
#define GET_FILE "get_file"
#define CMD_STATUS "cmd_status"
#define CMD_START "cmd_start"
#define CMD_STOP "cmd_stop"
#define GET_TREND_REAL "get_trend_real"
#define GET_TREND_MIN "get_trend_min"
#define GET_TREND_HOUR "get_trend_hour"
#define GET_TREND_UPL "get_trend_upl"
#define GET_RPM_TREND "get_rpm_trend"
#define GET_DIAG_TREND "get_diag_trend"
#define CMD_SEND_FILE "cmd_send_file"
#define GET_LOG_LIST "get_log_list"
#define GET_TIME "get_time"
#define GET_LOG_FILE "get_log_file"
#define CMD_ANALYSE "cmd_analyse"
#define CMD_DIAGNOSE "cmd_diagnose"
#define CMD_TREND_FREEZE "cmd_trend_freeze"
#define CMD_DELETE_FILE "cmd_delete_file"
#define GET_TCP_NOTE "get_tcp_note"
#define GET_COLL_LIST "get_coll_list"
#define GET_PARSER_DLL "get_parser_dll"
//#define GET_SYS_IDENT "get_sys_ident"
#define GET_BOARD_SERIAL "get_board_serial"
#define VMT_SET_SERIAL "set_vmt_serial"
#define GET_VMT_SERIAL "get_vmt_serial"

char vmtpath[MAX_PATH];
char *coll_name;
//char *pre_path;
char fl_name[MAX_PATH];//="";
char line[MAX_PATH];
struct logrec *logrec_array[100];
// ----- for vmt_set_serial --------------------------------
char eprom_buf[128]; // for reading 0-127 of EPROM (calib cft)
int got_vmt_key = 0;  // indicate that "get_vmt_key" was requested (protection)

int check_cmd_line(char *cmdline)
{
	char *p;
	char *p_end;

	p = cmdline;
	p_end = cmdline + strlen(cmdline);
 // get_coll_settings
	if (!strncmp(cmdline, GET_COLL_SETTINGS , strlen(GET_COLL_SETTINGS)) )
	{
	        // check if "get_vmt_serial" was requested before
		// to protect from older "umon" versions
    if (!got_vmt_key) 
		{
		 writelog("get_coll_settings: access denied !");
		 return -1;
		}
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("get_coll_settings: collector name not found!");
			return -1;
		}
		else
		{
			writelog("Found command: get_coll_settings");
			return 0;
		}
	}
 // get_last_file
	if (strncmp(cmdline, GET_LAST_FILE , strlen(GET_LAST_FILE)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("get_last_file: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(NULL, " ");
//		writelog("Found command: get_last_file");
		return 0;
	}
 // get_file
	if (strncmp(cmdline, GET_FILE , strlen(GET_FILE)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("get_file: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(NULL, " ");
		if (!cmd_args[1])
		{
			writelog("get_file: file name not found!");
			return -1;
		}
//		writelog("Found command: get_file");
		return 0;
	}
 // cmd_status
	if (strncmp(cmdline, CMD_STATUS , strlen(CMD_STATUS)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("cmd_status: collector name not found!");
			return -1;
		}
//		writelog("Found command: cmd_status");
		return 0;
	}
 // cmd_start
	if (strncmp(cmdline, CMD_START , strlen(CMD_START)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("cmd_start: collector name not found!");
			return -1;
		}
//		writelog("Found command: cmd_start");
		return 0;
	}
 // cmd_stop
	if (strncmp(cmdline, CMD_STOP , strlen(CMD_STOP)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("cmd_stop: collector name not found!");
			return -1;
		}
//		writelog("Found command: cmd_stop");
		return 0;
	}
 // get_time
	if (strncmp(cmdline, GET_TIME , strlen(GET_TIME)) == 0)
	{

//		writelog("Found command: get_time");
		return 0;
	}
 // get_trend_real
	if (strncmp(cmdline, GET_TREND_REAL , strlen(GET_TREND_REAL)) == 0)
	{
		cmd_args[0] = strtok(cmdline, "!");
		cmd_args[0] = strtok(NULL, "!");
		if (!cmd_args[0])
		{
			writelog("get_trend_real: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(cmd_args[0],"&");
		cmd_args[1] = strtok(NULL,"&");
		if (!cmd_args[1])
		{
			writelog("get_trend_real: date from not found!");
			return -1;
		}
		cmd_args[2] = strtok(NULL,"&");
		if (!cmd_args[2])
		{
			writelog("get_trend_real: date to not found!");
			return -1;
		}
//		writelog("Found command: get_trend_real");
		return 0;
	}
 // get_trend_min
	if (strncmp(cmdline, GET_TREND_MIN , strlen(GET_TREND_MIN)) == 0)
	{
		cmd_args[0] = strtok(cmdline, "!");
		cmd_args[0] = strtok(NULL, "!");
		if (!cmd_args[0])
		{
			writelog("get_trend_min: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(cmd_args[0],"&");
		cmd_args[1] = strtok(NULL,"&");
		if (!cmd_args[1])
		{
			writelog("get_trend_min: date from not found!");
			return -1;
		}
		cmd_args[2] = strtok(NULL,"&");
		if (!cmd_args[2])
		{
			writelog("get_trend_min: date to not found!");
			return -1;
		}
//		writelog("Found command: get_trend_min");
		return 0;
	}
 // get_trend_hour
	if (strncmp(cmdline, GET_TREND_HOUR , strlen(GET_TREND_HOUR)) == 0)
	{
		cmd_args[0] = strtok(cmdline, "!");
		cmd_args[0] = strtok(NULL, "!");
		if (!cmd_args[0])
		{
			writelog("get_trend_hour: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(cmd_args[0],"&");
		cmd_args[1] = strtok(NULL,"&");
		if (!cmd_args[1])
		{
			writelog("get_trend_hour: date from not found!");
			return -1;
		}
		cmd_args[2] = strtok(NULL,"&");
		if (!cmd_args[2])
		{
			writelog("get_trend_hour: date to not found!");
			return -1;
		}
//		writelog("Found command: get_trend_hour");
		return 0;
	}
 // get_trend_upl
	if (strncmp(cmdline, GET_TREND_UPL , strlen(GET_TREND_UPL)) == 0)
	{
		cmd_args[0] = strtok(cmdline, "!");
		cmd_args[0] = strtok(NULL, "!");
		if (!cmd_args[0])
		{
			writelog("get_trend_upl: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(cmd_args[0],"&");
		cmd_args[1] = strtok(NULL,"&");
		if (!cmd_args[1])
		{
			writelog("get_trend_upl: date from not found!");
			return -1;
		}
		cmd_args[2] = strtok(NULL,"&");
		if (!cmd_args[2])
		{
			writelog("get_trend_upl: date to not found!");
			return -1;
		}
//		writelog("Found command: get_trend_upl");
		return 0;
	}
 // get_rpm_trend
	if (strncmp(cmdline, GET_RPM_TREND , strlen(GET_RPM_TREND)) == 0)
	{
		cmd_args[0] = strtok(cmdline, "!");
		cmd_args[0] = strtok(NULL, "!");
		if (!cmd_args[0])
		{
			writelog("get_rpm_trend: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(cmd_args[0],"&");
		cmd_args[1] = strtok(NULL,"&");
		if (!cmd_args[1])
		{
			writelog("get_rpm_trend: date from not found!");
			return -1;
		}
		cmd_args[2] = strtok(NULL,"&");
		if (!cmd_args[2])
		{
			writelog("get_rpm_trend: date to not found!");
			return -1;
		}
//		writelog("Found command: get_rpm_trend");
		return 0;
	}
 // get_diag_trend
	if (strncmp(cmdline, GET_DIAG_TREND , strlen(GET_DIAG_TREND)) == 0)
	{
		cmd_args[0] = strtok(cmdline, "!");
		cmd_args[0] = strtok(NULL, "!");
		if (!cmd_args[0])
		{
			writelog("get_diag_trend: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(cmd_args[0],"&");
		cmd_args[1] = strtok(NULL,"&");
		if (!cmd_args[1])
		{
			writelog("get_diag_trend: date from not found!");
			return -1;
		}
		cmd_args[2] = strtok(NULL,"&");
		if (!cmd_args[2])
		{
			writelog("get_diag_trend: date to not found!");
			return -1;
		}
//		writelog("Found command: get_diag_trend");
		return 0;
	}
	if (strncmp(cmdline, CMD_SEND_FILE , strlen(CMD_SEND_FILE)) == 0)
	{
		return 0;
	}
 // get_log_list
	if (strncmp(cmdline, GET_LOG_LIST , strlen(GET_LOG_LIST)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("get_log_list: collector name not found!");
			return -1;
		}
//		writelog("Found command: get_log_list");
		return 0;
	}
 // get_log_file
	if (strncmp(cmdline, GET_LOG_FILE , strlen(GET_LOG_FILE)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("get_log_file: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(NULL, " ");
		if (!cmd_args[1])
		{
			writelog("get_log_file: file name not found!");
			return -1;
		}
//		writelog("Found command: get_log_file");
		return 0;
	}
 // cmd_analyse
	if (strncmp(cmdline, CMD_ANALYSE , strlen(CMD_ANALYSE)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("cmd_analyse: collector name not found!");
			return -1;
		}
//		writelog("Found command: cmd_analyse");
		return 0;
	}

 // cmd_diagnose
	if (strncmp(cmdline, CMD_DIAGNOSE , strlen(CMD_DIAGNOSE)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("cmd_diagnose: collector name not found!");
			return -1;
		}
//		writelog("Found command: cmd_diagnose");
		return 0;
	}
 // cmd_trend_freeze
	if (strncmp(cmdline, CMD_TREND_FREEZE , strlen(CMD_TREND_FREEZE)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("cmd_trend_freeze: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(NULL, " ");
		if (!cmd_args[1])
		{
			writelog("cmd_trend_freeze: freeze flag not found!");
			return -1;
		}
//		writelog("Found command: cmd_trend_freeze");
		return 0;
	}
 // cmd_delete_file
	if (strncmp(cmdline, CMD_DELETE_FILE , strlen(CMD_DELETE_FILE)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("cmd_delete_file: collector name not found!");
			return -1;
		}
		cmd_args[1] = strtok(NULL, " ");
		if (!cmd_args[1])
		{
			writelog("cmd_delete_file: file name not found!");
			return -1;
		}
//		writelog("Found command: cmd_delete_file");
		return 0;
	}
	//get_note_settings

	if (strncmp(cmdline, GET_NOTE_SETTINGS , strlen(GET_NOTE_SETTINGS)) == 0)
	{
//		writelog("Found command: get_note_settings");
		return 0;
	}
	// get_tcp_note
	if (strncmp(cmdline, GET_TCP_NOTE , strlen(GET_TCP_NOTE)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("get_tcp_note: collector name not found!");
			return -1;
		}
//		writelog("Found command: get_tcp_note");
		return 0;
	}

	// get_coll_list
	if (strncmp(cmdline, GET_COLL_LIST , strlen(GET_COLL_LIST)) == 0)
	{
//		writelog("Found command: get_coll_list");
		return 0;
	}

	if (strncmp(cmdline, GET_PARSER_DLL , strlen(GET_PARSER_DLL)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("get_parser_dll: collector name not found!");
			return -1;
		}
		return 0;
	}
	if (strncmp(cmdline, GET_BOARD_SERIAL , strlen(GET_BOARD_SERIAL)) == 0)
	{
		cmd_args[0] = strtok(cmdline, " ");
		cmd_args[0] = strtok(NULL, " ");
		if (!cmd_args[0])
		{
			writelog("get_board_serial: collector name not found!");
			return -1;
		}
		return 0;
	}
	if (strncmp(cmdline, VMT_SET_SERIAL , strlen(VMT_SET_SERIAL)) == 0)
	{
		return 0;
	}
	if (strncmp(cmdline, GET_VMT_SERIAL , strlen(GET_VMT_SERIAL)) == 0)
	{
		return 0;
	}
  sprintf(msgbuf,"Unknown command: <%s>",cmdline);
	writelog(msgbuf);
	return -1; // command not found
}
void do_command(char *cmdline, char *cmd_args[])
{
	getvmtpath(vmtpath,cmd_args[0],&pre_path,&coll_name);
	if ((!pre_path) | (!coll_name))
	{
		writelog("Error processing vmt_note.ini. Cannot continue...");
		exit(-1);
	}
	sprintf(line,"%s%s",pre_path,"vmt.ini");
	if (readvmtinifile(line,&coll_data)==-1)
		exit(-1);
	if (!strcmp(cmdline,GET_COLL_SETTINGS))
	{
        tmstamp("<get_coll_settings>");
		get_coll_settings();
		writelog("------end of <get_coll_settings> -------");
		return;
	}	
	if (!strcmp(cmdline,GET_FILE))
	{
        tmstamp("<get_file>");
		getfile(0);
		writelog("------end of <get_file> -----------");
		return;
	}	
	if (!strcmp(cmdline,GET_LAST_FILE))
	{
        tmstamp("<get_last_file>");
		get_last_file();
		writelog("------end of <get_last_file> -----------");
		return;
	}	
	if (!strcmp(cmdline,CMD_STATUS))
	{
        tmstamp("<cmd_status>");
		cmd_status();
		writelog("------end of <cmd_status> ----------");
		return;
	}	
	if (!strcmp(cmdline,CMD_START))
	{
        tmstamp("<cmd_start>");
		cmd_start();
		writelog("------end of <cmd_start> -------");
		return;
	}	
	if (!strcmp(cmdline,CMD_STOP))
	{
        tmstamp("<cmd_stop>");
		cmd_stop();
		writelog("------end of <cmd_stop> ---------");
		return;
	}	
	if (!strcmp(cmdline,GET_TIME))
	{
        tmstamp("<get_time>");
		get_time();
		writelog("------end of <get_time> -----------");
		return;
	}	
	if (!strcmp(cmdline,GET_TREND_REAL))
	{
        tmstamp("<get_trend_real>");
		get_trend_real();
		writelog("------end of <get_trend_real> --------");
		return;
	}	
	if (!strcmp(cmdline,GET_TREND_MIN))
	{
        tmstamp("<get_trend_min>");
		get_trend_min();
		writelog("------end of <get_trend_min> --------");
		return;
	}	
	if (!strcmp(cmdline,GET_TREND_HOUR))
	{
        tmstamp("<get_trend_hour>");
		get_trend_hour();
		writelog("------end of <get_trend_hour> --------");
		return;
	}	
	if (!strcmp(cmdline,GET_TREND_UPL))
	{
        tmstamp("<get_trend_upl>");
		get_trend_upl();
		writelog("------end of <get_trend_upl> --------");
		return;
	}	
	if (!strcmp(cmdline,GET_RPM_TREND))
	{
        tmstamp("<get_rpm_trend>");
		get_rpm_trend();
		writelog("------end of <get_rpm_trend> -------");
		return;
	}	
	if (!strcmp(cmdline,GET_DIAG_TREND))
	{
        tmstamp("<get_diag_trend>");
		get_diag_trend();
		writelog("------end of <get_diag_trend> -----");
		return;
	}	
	if (!strcmp(cmdline,CMD_SEND_FILE))
	{
        tmstamp("<cmd_send_file>");
		cmd_send_file();
		writelog("------end of <cmd_send_file> ------");
		return;
	}	
	if (!strcmp(cmdline,GET_LOG_LIST))
	{
        tmstamp("<get_log_list>");
		get_log_list();
		writelog("------end of <get_log_list> -------");
		return;
	}	
	if (!strcmp(cmdline,GET_LOG_FILE))
	{
        tmstamp("<get_log_file>");
		getfile(1);
		writelog("------end of <get_log_file> -------");
		return;
	}	
	if (!strcmp(cmdline,CMD_ANALYSE))
	{
        tmstamp("<cmd_analyse>");
		cmd_analyse();
		writelog("------end of <cmd_analyse> -------");
		return;
	}	
	if (!strcmp(cmdline,CMD_DIAGNOSE))
	{
        tmstamp("<cmd_diagnose>");
		cmd_diagnose();
		writelog("------end of <cmd_diagnose> --------");
		return;
	}	
	if (!strcmp(cmdline,CMD_TREND_FREEZE))
	{
        tmstamp("<cmd_trend_freeze>");
		cmd_trend_freeze();
		writelog("------end of <cmd_trend_freeze> -------");
		return;
	}	
	if (!strcmp(cmdline,CMD_DELETE_FILE))
	{
        tmstamp("<cmd_delete_file>");
		cmd_delete_file();
		writelog("------end of <cmd_delete_file> -------");
		return;
	}	
	if (!strcmp(cmdline,GET_NOTE_SETTINGS))
	{
        tmstamp("<get_note_settings>");
		get_note_settings();
		writelog("------end of <get_note_settings> ------");
		return;
	}	
	if (!strcmp(cmdline,GET_TCP_NOTE))
	{
        tmstamp("<get_tcp_note>");
		get_tcp_note();
		writelog("------end of <get_tcp_note> -------");
		return;
	}	
	if (!strcmp(cmdline,GET_COLL_LIST))
	{
        tmstamp("<get_coll_list>");
		get_coll_list();
		writelog("------end of <get_coll_list> -------");
		return;
	}	
	if (!strcmp(cmdline,GET_PARSER_DLL))
	{
        tmstamp("<get_parser_dll>");
		get_parser_dll();
		writelog("------end of <get_parser_dll> -------");
		return;
	}	
	if (!strcmp(cmdline,GET_BOARD_SERIAL))
	{
        tmstamp("<get_board_serial>");
		get_board_serial();
		writelog("------end of <get_board_serial> -------");
		return;
	}	
	if (!strcmp(cmdline,VMT_SET_SERIAL))
	{
        tmstamp("<vmt_set_serial>");
		set_vmt_serial();
		writelog("------end of <vmt_set_serial> -------");
		return;
	}	
	if (!strcmp(cmdline,GET_VMT_SERIAL))
	{
    tmstamp("<get_vmt_serial>");
		get_vmt_serial();
		writelog("------end of <get_vmt_serial> -------");
		return;
	}	
}
// write to log with timestamp
void tmstamp(char *s)
{
 time_t tt;
 char *p;
 
 tt=time(NULL);
 snprintf(msgbuf,MAX_LINE_LEN-1,"%s%s",ctime(&tt),s);
 p=strchr(msgbuf,'\n'); // ctimes makes a string with '\n', change to "space"
 if (p) 
  *p=' ';
 writelog(msgbuf);
}
int getfile(int need_pack)
{
	char *p=NULL;
	char tmp_path[MAX_LINE_LEN]="\0";
	int res;
	int fd = -1;
	unsigned long fl_len,fl_len_net;
	unsigned long crc=0;
	struct stat st;
	FILE *tmp = NULL;
	FILE *src = NULL;

	if (!cmd_args[1])
		return -1;
	correct_case(cmd_args[1]);	
	/*
	//change '\' to '/'	
	while (p=strchr(cmd_args[1], '\\'),p)
		*p='/';
	// correct case of filename	
	if (p=(char*)strcasestr(cmd_args[1],"trends"),p)
		memcpy(p,"Trends",6);
	if (p=(char*)strcasestr(cmd_args[1],"minuts"),p)
		memcpy(p,"Minuts",6);
	if (p=(char*)strcasestr(cmd_args[1],"hours"),p)
		memcpy(p,"Hours",5);
	if (p=(char*)strcasestr(cmd_args[1],"rpms"),p)
		memcpy(p,"Rpms",4);
	if (p=(char*)strcasestr(cmd_args[1],"bands"),p)
		memcpy(p,"Bands",5);
	if (p=(char*)strcasestr(cmd_args[1],"dlevels"),p)
		memcpy(p,"dlevels",7);
	if (p=(char*)strcasestr(cmd_args[1],"appr"),p)
		memcpy(p,"Appr",4);
	if (p=(char*)strcasestr(cmd_args[1],"matrix"),p)
		memcpy(p,"matrix",6);
	if (p=(char*)strcasestr(cmd_args[1],"levels"),p)
		memcpy(p,"levels",6);
	if (p=(char*)strcasestr(cmd_args[1],"vmt"),p)
		memcpy(p,"vmt",3);
	*/

	if (!strcasecmp(cmd_args[0],"NULL"))
		strcpy(tmp_path,"../");
	else
		strcpy(tmp_path,pre_path); 

	snprintf(&fl_name[0],MAX_PATH-1,"%s%s",tmp_path,cmd_args[1]); 
	 // if packing needed, create temp file
	if (need_pack)
	{
    src=fopen(fl_name, "r"); 
    if (!src)
    {
		snprintf(msgbuf,MAX_LINE_LEN-1,"get_file: cannot open file! <%s>",fl_name );
        writelog(msgbuf);
		res = BAD_STATUS;
		write(0,&res,sizeof(res));
		return res;
    }
		tmp=tmpfile(); // create temp file for storing compressed data
		if (!tmp)
		{
			sprintf(msgbuf,"get_file: cannot create temp file <errno=%d>!!",errno);
			writelog(msgbuf);
			fclose(src);
			res = BAD_STATUS;
			write(0,&res,sizeof(res));
			return res;
		}
	    // do compression
		res = def(src, tmp, Z_DEFAULT_COMPRESSION);
		if (res != Z_OK)
		{
			sprintf(msgbuf,"get_file: packing error!! errcode=%d",res);
			writelog(msgbuf);
			close(fd);
			fclose(tmp);
			res = BAD_STATUS;
			write(0,&res,sizeof(res));
			return -1;
		}     	 
    // zipping is done, reading data to buffer
    fl_len=ftell(tmp);
		p=calloc(fl_len+1,sizeof(char));
		rewind(tmp);
		if (fread(p,1,fl_len,tmp) == -1)
		{
			writelog("cannot read from temp file:  ");
			res=BAD_STATUS;
			write(0,&res,sizeof(res));
			free(p);
			fclose(src);
			fclose(tmp);
			return res;
		}       
		fclose(tmp);
		fclose(src);
	} // if (needpack)  
	else
	{ 
	 fd=open(fl_name,O_RDONLY);
	 if (fd == -1)
	 {
		snprintf(msgbuf,MAX_LINE_LEN-1,"get_file: cannot open file! <%s>",fl_name );
        writelog(msgbuf);
		res = BAD_STATUS;
		write(0,&res,sizeof(res));
		return res;
	 }
		if (fstat(fd,&st)!=0)
		{
			sprintf(msgbuf, "fstat error:  %d",errno);
			writelog(msgbuf);
			res=BAD_STATUS;
			write(0,&res,sizeof(res));
			close(fd);
			return res;
		}
		fl_len = st.st_size;
		p=calloc(fl_len+1,sizeof(char));
		if (read(fd,p,fl_len)==-1)
		{
			sprintf(msgbuf, "cannot read from file:  %s",fl_name);
			writelog(msgbuf);
			res=BAD_STATUS;
			write(0,&res,sizeof(res));
			free(p);
			close(fd);
			return res;
		}
		close(fd);
	}	 
	res=0;
	write(0,&res,sizeof(res));
	fl_len_net=htonl(fl_len);
 // sending length of data
	write(0,&fl_len_net,sizeof(fl_len_net));
	write(0,p,fl_len);
	crc_init();
 // calculate and send CRC32
	crc=crc_cycle(crc,p,fl_len);
	crc=htonl(crc);
	write(0,&crc,sizeof(crc));
	if (p)
		free(p);
	return 0;

}
int get_last_file()
{
	int hMapped;
	char *pMapped,*p;
	int res;
	char trend_fl_name[20]="\0";
	unsigned char len_flname;

	hMapped=shmget(VMT_NOTE_KEY + coll_data.adc_slot,VMT_NOTE_LEN,0);
	if (hMapped==-1)
	{
		res=BAD_STATUS;
		writelog("shmget error");
		write(0,&res,sizeof(res));
		return -1;
	}

	pMapped=shmat(hMapped,NULL,SHM_RDONLY);
	if (pMapped==(void*)-1)
	{
		res=BAD_STATUS;
		writelog("shmat error ");
		write(0,&res,sizeof(res));
		return -1;

	}
	p=pMapped;
	p += 16+8; // AggrName:String[15] + MTime:TDateTime
	len_flname = (unsigned char)(*p);
	p++;
	strncpy(trend_fl_name,p,len_flname);
	snprintf(msgbuf,MAX_LINE_LEN-1,"Last trend file <%s>",trend_fl_name);
	writelog(msgbuf);
	// create command get_file with "trend_fl_name"
	if (!cmd_args[0])
	 snprintf(cmdline,sizeof(cmdline),"get_file J103 Trends/%s",trend_fl_name);
	else
   snprintf(cmdline,sizeof(cmdline), "get_file %s Trends/%s",cmd_args[0], trend_fl_name); 
	check_cmd_line(cmdline);
	do_command(cmdline, cmd_args);
	if (pMapped)
		shmdt(pMapped);
	return 0;
}
int get_coll_settings()
{
	int res;
//	char fl_name[MAX_PATH]="";
//	char line[100];
	FILE *fl;
	int line_cnt;

	res=0;
	write(0,&res,sizeof(res));
	sprintf(&fl_name[0],"%s%s",pre_path,"vmt.ini");
	fl=fopen(fl_name,"r");
	if (fl)
	{
		line_cnt=0;
		while (fgets(line,sizeof(line),fl))
			line_cnt++;
		line_cnt=htonl(line_cnt);
		write(0,&line_cnt,sizeof(line_cnt));
		rewind(fl);
		while (fgets(line,sizeof(line),fl))
		{
//		 strcat(line,"\n\r");
			write(0, line,strlen(line));
		}
		fclose(fl);
	}
	sprintf(&fl_name[0],"%s%s",pre_path,"levels.ini");
	fl=fopen(fl_name,"r");
	if (fl)
	{
		line_cnt=0;
		while (fgets(line,sizeof(line),fl))
			line_cnt++;
		line_cnt=htonl(line_cnt);
		write(0,&line_cnt,sizeof(line_cnt));
		fseek(fl, 0, SEEK_SET);
		while (fgets(line,sizeof(line),fl))
		{
//		 strcat(line,"\n\r");
			write(0,line,strlen(line));
		}
		fclose(fl);
	}
	return 0;
}
int cmd_status()
{
	int status;

    status=BAD_STATUS;
	switch (servicegetstatus("vmt"))
	{
		case SVC_RUNNING:{ status=0;writelog("cmd_status: working!");break;}
		case SVC_STOPPED:{ status=htonl(1);writelog("cmd_status: stopped!");break;}
		case SVC_NOTDEF: { status=BAD_STATUS;writelog("cmd_status: unknown!");break;}
	}
	write(0,&status,sizeof(status));
	return 0;
}
int cmd_start()
{
	int status,errcode,changed;
	char cdir[256];
	char *dir_path;
	char *note_buf;
	int fail=0;
	FILE *f_note_pid;
	char note_pid[10];
	unsigned char len;
	double mtime;
	char err_fl_name[10]="";
	char *p;
	FILE *ff;

	rewrite_time_stamp(&changed);
	if (changed > 0) // if config was changed
	{
		dir_path=calloc(MAX_PATH,sizeof(char));
		snprintf(dir_path,MAX_PATH-1,"%sTrends",pre_path);    
		fail=fail|clear_dir(dir_path); 
		snprintf(dir_path,MAX_PATH-1,"%sMinuts",pre_path);    
		fail=fail|clear_dir(dir_path); 
		snprintf(dir_path,MAX_PATH-1,"%sRpms",pre_path);    
		fail=fail|clear_dir(dir_path); 
		snprintf(dir_path,MAX_PATH-1,"%sHours",pre_path);    
		fail=fail|clear_dir(dir_path); 
		if (fail)
			writelog("Some files were not deleted!");   
		else  
			writelog("All directories were cleared !");   
		free(dir_path);  
	}
	
 // switch current dir to VMT
	getcwd(cdir,sizeof(cdir));
	chdir(ROOT_PROG_PATH);
	status=servicestart("vmt",&errcode); // try to start VMT
	status=htonl(status);
	write(0,&status,sizeof(status)); // send status
	chdir(cdir); // change back current dir 

	return 0;
}
int cmd_stop()
{
	int status,errcode;
	char *note_buf,*p;
	unsigned char len;
	double mtime;
	char err_fl_name[10]="";
	FILE *ff;

	status=servicestop("vmt");
	errcode=status;
	status=htonl(status);
	write(0,&status,sizeof(status));
 // Now create UDP notyfication and send to operators
 
	note_buf=calloc(MAX_LINE_LEN,sizeof(char));
	len = strlen(coll_name);
	p=note_buf;
	/*(unsigned char)*/*p=len;p++; // length of aggr name
	strcpy(p,coll_name);p += 15; // aggr name
	mtime = unix_to_datetime(time(NULL)); // mtime
	memcpy(p,&mtime,sizeof(mtime));p +=  sizeof(mtime);
	snprintf(err_fl_name,sizeof(err_fl_name),"%d.err",errcode); // start error code
	len=strlen(err_fl_name);
	*p=len;p++;
	strncat(p,err_fl_name,sizeof(err_fl_name));p +=  sizeof(err_fl_name);	
	/* -------------------------
	ff=fopen("/usr/local/Hercules/inetd/note.tmp","a+");
	fwrite(note_buf,1,(int)(p-note_buf),ff);
	fclose(ff);
	-------------------------------------------*/
 // send UDP to operators
	send_udp_note(note_buf , (int)(p-note_buf));
	free(note_buf);
	return 0;
}

int get_trend_real()
{
	time_t dt_from,dt_to;    // data interval variables
	int status;
	char *path=NULL;
	unsigned long data_len;
	char *buf=NULL;

	dt_from=parsedata(cmd_args[1]);
	if (dt_from == -1)
	{
		sprintf(msgbuf,"get_trend_real: error converting date/time: %s",cmd_args[1]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	dt_to=parsedata(cmd_args[2]);
	if (dt_to == -1)
	{
		sprintf(msgbuf,"get_trend_real: error converting date/time: %s",cmd_args[2]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	F=tmpfile();
//    F=fopen("trend.tmp","w+");
	if (!F)
	{
		writelog("get_trend_real: cannot create temp file !");  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	}
	path=calloc(MAX_PATH+1,sizeof(char));
	snprintf(path,MAX_PATH,"%sTrends",pre_path);
//	writelog(path);
	if (processtrend(path,".SHT",dt_from,dt_to,WM_MONITOR)) 
	{
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
	}
	else
	{
		status=0;
		write(0,&status,sizeof(status));
		data_len=htonl(ftell(F)); // send length of stream
		write(0,&data_len,sizeof(data_len));

		fflush(F);
		rewind(F);
		buf=calloc(1024,sizeof(char));
		while (!feof(F))
		{
			fread(buf,1,sizeof(buf),F); // sending data
			write(0,buf,sizeof(buf));   // block by block
		}
	}
	fclose(F);
	if (path)
		free(path);
	if (buf)
		free(buf); 
	return 0;
}
int get_trend_min()
{
	time_t dt_from,dt_to;    // data interval variables
	int status;
	char *path=NULL;
	unsigned long data_len;
	char *buf=NULL;
	dt_from=parsedata(cmd_args[1]);
	if (dt_from == -1)
	{
		sprintf(msgbuf,"get_trend_min: error converting date/time: %s",cmd_args[1]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	dt_to=parsedata(cmd_args[2]);
	if (dt_to == -1)
	{
		sprintf(msgbuf,"get_trend_min: error converting date/time: %s",cmd_args[2]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	F=tmpfile();
	if (!F)
	{
		writelog("get_trend_min: cannot create temp file !");  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	}
	path=calloc(MAX_PATH+1,sizeof(char));
	snprintf(path,MAX_PATH,"%sMinuts",pre_path);
//	writelog(path);
	if (processtrend(path,".SHM",dt_from,dt_to,WM_MONITOR)) 
	{
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
	}
	else
	{
		status=0;
		write(0,&status,sizeof(status));
		data_len=htonl(ftell(F)); // send length of stream
		write(0,&data_len,sizeof(data_len));

		fflush(F);
		rewind(F);
		buf=calloc(1024,sizeof(char));
		while (!feof(F))
		{
			fread(buf,1,sizeof(buf),F); // sending data
			write(0,buf,sizeof(buf));   // block by block
		}
	}
	fclose(F);
	if (path)
		free(path);
	if (buf)
		free(buf); 
	return 0;
}
int get_trend_hour()
{
	time_t dt_from,dt_to;    // data interval variables
	int status;
	char *path=NULL;
	unsigned long data_len;
	char *buf=NULL;
	dt_from=parsedata(cmd_args[1]);
	if (dt_from == -1)
	{
		sprintf(msgbuf,"get_trend_hour: error converting date/time: %s",cmd_args[1]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	dt_to=parsedata(cmd_args[2]);
	if (dt_to == -1)
	{
		sprintf(msgbuf,"get_trend_hour: error converting date/time: %s",cmd_args[2]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	F=tmpfile();
	if (!F)
	{
		writelog("get_trend_hour: cannot create temp file !");  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	}
	path=calloc(MAX_PATH+1,sizeof(char));
	snprintf(path,MAX_PATH,"%sHours",pre_path);
//	writelog(path);
	if (processtrend(path,".SHH",dt_from,dt_to,WM_MONITOR)) 
	{
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
	}
	else
	{
		status=0;
		write(0,&status,sizeof(status));
		data_len=htonl(ftell(F)); // send length of stream
		write(0,&data_len,sizeof(data_len));

		fflush(F);
		rewind(F);
		buf=calloc(1024,sizeof(char));
		while (!feof(F))
		{
			fread(buf,1,sizeof(buf),F); // sending data
			write(0,buf,sizeof(buf));   // block by block
		}
	}
	fclose(F);
	if (path)
		free(path);
	if (buf)
		free(buf); 
	return 0;
}
int get_trend_upl()
{
	time_t dt_from,dt_to;    // data interval variables
	int status;
	char *path=NULL;
	unsigned long data_len;
	char *buf=NULL;
	dt_from=parsedata(cmd_args[1]);
	if (dt_from == -1)
	{
		sprintf(msgbuf,"get_trend_upl: error converting date/time: %s",cmd_args[1]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	dt_to=parsedata(cmd_args[2]);
	if (dt_to == -1)
	{
		sprintf(msgbuf,"get_trend_upl: error converting date/time: %s",cmd_args[2]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	F=tmpfile();
	if (!F)
	{
		writelog("get_trend_upl: cannot create temp file !");  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	}
	path=calloc(MAX_PATH+1,sizeof(char));
	snprintf(path,MAX_PATH,"%sTrends",pre_path);
//	writelog(path);
	if (processtrend(path,".SHU",dt_from,dt_to,WM_MONITOR)) 
	{
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
	}
	else
	{
		status=0;
		write(0,&status,sizeof(status));
		data_len=htonl(ftell(F)); // send length of stream
		write(0,&data_len,sizeof(data_len));

		fflush(F);
		rewind(F);
		buf=calloc(1024,sizeof(char));
		while (!feof(F))
		{
			fread(buf,1,sizeof(buf),F); // sending data
			write(0,buf,sizeof(buf));   // block by block
		}
	}
	fclose(F);
	if (path)
		free(path);
	if (buf)
		free(buf); 
	return 0;
}
int get_rpm_trend()
{
	time_t dt_from,dt_to;    // data interval variables
	int status;
	char *path=NULL;
	unsigned long data_len;
	char *buf=NULL;


	dt_from=parsedata(cmd_args[1]);
	if (dt_from == -1)
	{
		sprintf(msgbuf,"get_rpm_trend: error converting date/time: %s",cmd_args[1]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	dt_to=parsedata(cmd_args[2]);
	if (dt_to == -1)
	{
		sprintf(msgbuf,"get_rpm_trend: error converting date/time: %s",cmd_args[2]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	F=tmpfile();
	if (!F)
	{
		writelog("get_rpm_trend: cannot create temp file !");  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	}
	path=calloc(MAX_PATH+1,sizeof(char));
	snprintf(path,MAX_PATH,"%sRpms",pre_path);
//	writelog(path);
	if (processtrend(path,".SHT",dt_from,dt_to,WM_MONITOR)) 
	{
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
	}
	else
	{
		status=0;
		write(0,&status,sizeof(status));
		data_len=htonl(ftell(F)); // send length of stream
		write(0,&data_len,sizeof(data_len));

		fflush(F);
		rewind(F);
		buf=calloc(1024,sizeof(char));
		while (!feof(F))
		{
			fread(buf,1,sizeof(buf),F); // sending data
			write(0,buf,sizeof(buf));   // block by block
		}
	}
	fclose(F);
	if (path)
		free(path);
	if (buf)
		free(buf); 
	return 0;
}
int get_diag_trend()
{
	time_t dt_from,dt_to;    // data interval variables
	int status;
	char *path=NULL;
	unsigned long data_len;
	char *buf=NULL;

	dt_from=parsedata(cmd_args[1]);
	if (dt_from == -1)
	{
		sprintf(msgbuf,"get_diag_trend: error converting date/time: %s",cmd_args[1]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	dt_to=parsedata(cmd_args[2]);
	if (dt_to == -1)
	{
		sprintf(msgbuf,"get_diag_trend: error converting date/time: %s",cmd_args[2]);
		writelog(msgbuf);  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	} 
	F=tmpfile();
//    F=fopen("trend.tmp","w+");
	if (!F)
	{
		writelog("get_diag_trend: cannot create temp file !");  
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
		return -1;
	}

	path=calloc(MAX_PATH+1,sizeof(char));
	snprintf(path,MAX_PATH,"%sDiagnoses",pre_path);
	if (processtrend(path,".SHD",dt_from,dt_to,WM_DIAGNOSE)) 
	{
		status=BAD_STATUS;
		write(0,&status,sizeof(status));
	}
	else
	{
		status=0;
		write(0,&status,sizeof(status));
		data_len=htonl(ftell(F)); // send length of stream
		write(0,&data_len,sizeof(data_len));

		fflush(F);
		rewind(F);
		buf=calloc(1024,sizeof(char));
		while (!feof(F))
		{
			fread(buf,1,sizeof(buf),F); // sending data
			write(0,buf,sizeof(buf));   // block by block
		}
	}
	fclose(F);
	if (path)
		free(path);
	if (buf)
		free(buf); 
	return 0;
}

int get_time()
{
	time_t unix_time;
	double delphi_dt;

	unix_time=time(NULL);
	delphi_dt=unix_to_datetime(unix_time);
	write(0,&delphi_dt,sizeof(delphi_dt));
//	writelog("get_time done.");
	return 0;
}

// recives file from client program, and place it into collector folder
// or into notification service folder
int cmd_send_file()
{
	char *buf,*p; // buffer for target file path
	char tmp[50]="";
	char fl_name[MAX_LINE_LEN]="";   // name of the collector, name of the target file
	char tempname[]="tmp.XXXXXX";
	int ans,i;      // auxiliary vars
	unsigned long bytes_read,total_bytes,fl_len;
	int temp_fd;

// format of the command:
// cmd_send_file
// receive 1 byte (if "0" - all OK)
// <collector_name> for collector or <null> for notificator
// receive 1 byte (if "0" - all OK)
// <file_name>
// receive 1 byte (if "0" - all OK)
// send stream with file
// receive 1 byte (if "0" - all OK)

	ans=0;

	write(0,&ans,sizeof(ans));
	read(0,tmp,sizeof(tmp)); // get machine name
	tmp[strlen(tmp)-2]='\0'; // remove ENTER
	for (i=0;i<strlen(tmp);i++)    // machine name to upper case
		tmp[i]=toupper(tmp[i]);
	if (strstr(tmp,"NULL")) // check the path
		strcpy(fl_name,"../");     
	else
	{
	 getvmtpath(vmtpath,tmp,&pre_path,&coll_name);
	 strcpy(fl_name,pre_path);       
	} 
	write(0,&ans,sizeof(ans));
	
	read(0,tmp,sizeof(tmp)); // get file name
	tmp[strlen(tmp)-2]='\0'; // remove ENTER
	correct_case(tmp);
	/*
	// check name of the file and correct case if needed
	if (p=(char*)strcasestr(tmp,"bands"),p)
		memcpy(p,"Bands",5);
	if (p=(char*)strcasestr(tmp,"dlevels"),p)
		memcpy(p,"dlevels",7);
	if (p=(char*)strcasestr(tmp,"appr"),p)
		memcpy(p,"Appr",4);
	if (p=(char*)strcasestr(tmp,"matrix"),p)
		memcpy(p,"matrix",6);
	if (p=(char*)strcasestr(tmp,"vmt"),p)
		memcpy(p,"vmt",3);
	if (p=(char*)strcasestr(tmp,"levels"),p)
		memcpy(p,"levels",6);
	 */
	strcat(fl_name,tmp);
	// change '\' to '/' !!
	for (i=0;i<strlen(fl_name);i++)  
	 if (fl_name[i] == '\\')
	  fl_name[i]='/';
	// -------------------------------  

//      writelog(fl_name);	

/*
	bFoundMatch:=False;// now check extension of the file
	for i:=Low(allowed_ext)  to High(allowed_ext) do
	 if allowed_ext[i] = UpperCase(ExtractFileExt(FlName)) then
	 begin
		bFoundMatch:=True;
		break;
	 end;
	if not bFoundMatch then  // extension are disallowed
	 raise Exception.Create(msg_BadFileName+FlNAme);
  */  
	ans=0;
	write(0,&ans,sizeof(ans));

	read(0,&fl_len,sizeof(fl_len)); // get length of input data
	fl_len=ntohl(fl_len);
	temp_fd=mkstemp(tempname);
	if (temp_fd==-1)
	{
		writelog("cmd_send_file: cannot create temp file (mkstemp error)!!") ;
		ans=BAD_STATUS;
		write(0,&ans,sizeof(ans));
		return -1;
	}
	buf=calloc(1024,sizeof(char));
    // receiving data
        total_bytes = 0;
	while (total_bytes < fl_len)
	{
		bytes_read = read(0,buf,1024);
                if (bytes_read == -1)
		 break;
		write(temp_fd,buf,bytes_read);   
                total_bytes += bytes_read;
	}
	free(buf); // free buffer and
	close(temp_fd); // close temp file
	if (bytes_read == -1) // check for read error
	{
		writelog("cmd_send_file: cannot read input file!!") ;
		ans=BAD_STATUS;
		write(0,&ans,sizeof(ans));
		unlink(tempname); // del tempfile
		return -1;
	}    
    // read data success, write or re-write target file
    // We cannot use rename() becouse tempname and fl_name
    // can be on different partitions. 
    // So do move_file()
	if (move_file(tempname,fl_name))
	{
		writelog("cmd_send_file: cannot rewrite file!!") ;
		ans=BAD_STATUS;
		write(0,&ans,sizeof(ans));
		unlink(tempname); // del tempfile
		return -1;
	}
    // report OK
	ans=0;
	write(0,&ans,sizeof(ans));
	writelog("cmd_send_file: delivered success !!");
	unlink(tempname); // del tempfile
	return 0;
}

int get_log_list()
{

	char *log_path;
	char *fl_name;
	char *buf;
	DIR *log_dir;
	struct dirent *dir_entry;
	struct logrec *loglist;
	struct logrec *prev,*first,*max_time_rec;
	time_t max_time;
	struct stat buf_st;
	unsigned int n_rec = 0;
	unsigned int n_rec1;
	int i;
	int  dt;
	unsigned char fl_name_len;
	unsigned long strm_len,n_read;
	int ans;
	int  stream_size;

 // format :
 // get_log_list <collector_name> or get_log_list <null> for logs of notification service
 // the answer is :
 // N:Integer - how much files was found, -1 if any error, -2 incorrect collector's name
 // after that :
 // <creation_date/time><file_name>...<creation_date/time><file_name>
 // where: <creation_date> TDateTime ; <file_name> String[16]
//	F=NULL;
	log_path=calloc(MAX_PATH,sizeof(char));
	fl_name=calloc(MAX_PATH,sizeof(char));
	if (strcasecmp(cmd_args[0],"NULL")) // check the path
		strcpy(log_path,pre_path);     
	else
		strcpy(log_path,"../");       

	sprintf(msgbuf, "get_log_list: start searching log files in <%s>",log_path);    
	writelog(msgbuf);

	log_dir=opendir(log_path);
	if (!log_dir)
	{
		snprintf(msgbuf,sizeof(msgbuf), "get_log_list: cannot open directory <%s>",log_path);    
		writelog(msgbuf);
		ans=BAD_STATUS;
		write(0,&ans,sizeof(ans));
		free(log_path);
		free(fl_name);
		return -1;         
	}
//	prev=NULL;
//	first=NULL;
	while (dir_entry=readdir(log_dir),dir_entry)
	{
		if (strcasestr(dir_entry->d_name,".LOG")||strcasestr(dir_entry->d_name,".~LOG"))// it is *.log file
		{
			loglist=calloc(1,sizeof(struct logrec)); // create new record
			/*
			if (!first)
			{
				first=loglist;
				first->prev=NULL;
			} 
			*/
			strcpy(loglist->fname,dir_entry->d_name); // write name of log file
			snprintf(fl_name,MAX_PATH-1,"%s%s",log_path,loglist->fname); // full name
			if (stat(fl_name,&buf_st) == 0)
				loglist->ftime=buf_st.st_mtime; // if stat OK, get the time of file
			else
			{
				sprintf(msgbuf,"get_log_list stat error: file <%s> !",dir_entry->d_name);      
				writelog(msgbuf);
				free(loglist); // if error, don't add record to list
				continue;
			}
			logrec_array[n_rec] = loglist;
			sprintf(msgbuf," Found log file <%s> ",dir_entry->d_name);
			writelog(msgbuf);
			n_rec++; // increase log records counter
		}    
	}
	sprintf(msgbuf, "%d log file(s) found",n_rec);    
	writelog(msgbuf);

 // list is ready, now sort and send
  qsort(logrec_array,n_rec,sizeof(struct logrec *),fn_logcmp);
  n_rec1 = htonl(n_rec);
  write(0,&n_rec1,sizeof(n_rec1)); 
  stream_size=htonl(n_rec * sizeof(struct logrec));
  // send size of data
  write(0,&stream_size,sizeof(stream_size)); 
  for (i=0;i<n_rec;i++)
  {
    // write data to output stream
		dt=unixfiletime_to_winfiletime(logrec_array[i]->ftime);// - UNIX_SEC_DELTA;
		write(0,&dt,sizeof(dt));
		fl_name_len=strlen(logrec_array[i]->fname);
		write(0,&fl_name_len,sizeof(fl_name_len));
		write(0,logrec_array[i]->fname,sizeof(max_time_rec->fname));
    // delete record from list
		free(logrec_array[i]);
	} // for (i=0;i<n_rec;i++)
	closedir(log_dir); 
	free(log_path);
	free(fl_name);
	return 0; 
}
int fn_logcmp(const void *p, const void *p1)
{
    struct logrec **log;
    struct logrec **log1;
    
    log = (struct logrec **)p;
    log1 = (struct logrec **)p1;
    if ((*log)->ftime > (*log1)->ftime)
     return -1;
    else 
     if ((*log)->ftime < (*log1)->ftime)
      return 1;
     else
      return 0; 
}
int get_log_file()
{
//        writelog("get_log_file");
	getfile(1);
	return 0;
}

// start analyse mode of collector
int cmd_analyse()
{
#ifdef ANALYSE
	int i;          // auxiliary vars
	FILE *inp_f;               // stream to receive request data
	char *buf;                 // buffer for request and answer data
	struct stat st;
	int req_len; // size of "request.bin"
	char c_dir[MAX_LINE_LEN];
	unsigned long bytes_read,total_read;

#endif
	int res;
 // format:
 // cmd_analyse <collector_name>
 // read answer (Integer)
 //  0 - all OK
 //  -1 - any error (see vmt_note.log)
 //  -2 - incorrect collector's name
 //  -3 - analyse mode is already started
 //  -100 - request accepted, wait for result
 // WriteStream(InputFileStream); send the "request.bin" file
 // read answer (ReadInteger)
 //  -1 - any error (or timeout expired)
 //  -100 - request accepted, wait for result
 //  0 - all Ok
 // do ReadStream with analyse data ("result.bin")


#ifdef ANALYSE
	res=-1;
	getcwd(c_dir,sizeof(c_dir));
	if (makelock(ANALYSE_LCK)==-1)
	{
		res=htonl(-3);
		write(0,&res,sizeof(res));
		writelog("Analysing already started !");
		return -3;
	}
	else
		writelog("Starting analyse session...");

	buf=calloc(1024,sizeof(char));

	snprintf(buf,1023,"%sanalyse",pre_path);
	mkdir(buf,511); // create "analyse" directory with mode Ox777
	if (chdir(buf)==-1)
	{
		snprintf(msgbuf,MAX_LINE_LEN-1,"cmd_analyse: cannot change to <%s> folder!",buf);
		writelog(msgbuf);
		res=BAD_STATUS;
		write(0,&res,sizeof(res));
		free(buf);
		removelock(ANALYSE_LCK);
		return -1;
	}     
	unlink("request.bin.tmp"); // delete prev file if exists
	inp_f = fopen("request.bin.tmp","w"); // open "request.bin" file
	if (inp_f)
	{
		res=0;
		write(0,&res,sizeof(res)); // read request file from stdin
		read(0,&req_len,sizeof(req_len)); // get stream size
		req_len=ntohl(req_len);
		total_read=0;
		while (total_read < req_len)
		{ 
		        bytes_read = read(0,buf,1024);
			if (bytes_read==-1)
			 break;
			fwrite(buf,1,bytes_read,inp_f);   
			total_read += bytes_read;
		}	 
		fclose(inp_f); // close temp file
		if (bytes_read == -1) // check for read error
		{
			writelog("cmd_analyse: cannot read 'request.bin' file!!") ;
			res=BAD_STATUS;
			write(0,&res,sizeof(res));
			free(buf);
			chdir(c_dir);
			unlink("request.bin.tmp"); // del tempfile
			removelock(ANALYSE_LCK);
			return -1;
		}    
 // ---------------- delete previous files (*.tmp is temporary)-------------------------------
		unlink("request.bin");
		unlink("result.bin");
		unlink("result.bin.tmp");
 // ----------------------------------------------------------------------
 //  rename temporary file
		rename("request.bin.tmp","request.bin");
		writelog("cmd_analyse:request.bin saved OK!");
 // start waiting for result file
		res=-1;
		i = 1; // wait counter
		while (i <= ANALYSE_TMOUT) 
//wait cycle
		{
			if (stat("result.bin",&st) != -1) // if result file exists
			{
				writelog("Analysing finished !");
				if (!cmd_args[0])
				 strcpy(cmdline,"get_file J103 analyse/result.bin");
				else
         snprintf(cmdline,sizeof(cmdline),"get_file %s analyse/result.bin",cmd_args[0]); 
				check_cmd_line(cmdline);
//				res=0;
//				write(0,&res,sizeof(res)); // send reply
				// do getfile to send "result.bin"
				if (!getfile(0))// send file to client
				 writelog("File <result.bin> was sent successfully");
				else
				 writelog("Error sending file <result.bin>");
	
				// clearing and freeing
				unlink("result.bin");
				free(buf);
				removelock(ANALYSE_LCK);
				chdir(c_dir); // get back 
				return 0;
			}
			sleep(1); // minimize CPU load
			res=htonl(-100);
			if (write(0,&res,sizeof(res)) == -1) // continue waiting
			{
		         // connection lost...
		         res=BAD_STATUS;
		         free(buf);
		         chdir(c_dir);
		         removelock(ANALYSE_LCK);
		         writelog("cmd_analyse: connection lost !");
		         return -1;
			}
			snprintf(msgbuf,MAX_LINE_LEN-1,"Waiting for result file: %d sec...",i);
			writelog(msgbuf);
			i++;
		}
		// time expired...
		free(buf);
		chdir(c_dir);
		removelock(ANALYSE_LCK);
		writelog("cmd_analyse: timeout expired !");
		return -1;

	}
	else // if cannot create temp file
	{
		writelog("cmd_analyse: cannot create <request.bin.tmp>");
		res=BAD_STATUS;
		write(0,&res,sizeof(res));
		free(buf);
		removelock(ANALYSE_LCK);
		return -1;
	}    

// ----------------------------------------------------------------------
#else
// analyse mode is switched off
	writelog("CMD_ANALYSE command is not supported.");
	res=BAD_STATUS;
	write(0,&res,sizeof(res));
	return -1;
#endif
}
int cmd_diagnose()
{
//   	 int n_aggr:Integer;  // auxiliary vars
	int hfilemapobj; // handle for file mapping
//	int  *sharedbuf;  // pointer to file mapping buffer
	struct men_record  *sharedbuf;  // pointer to file mapping buffer
    
	int tm_wait;     // time counter (while waiting for result)
	struct stat st;
	char c_dir[MAX_LINE_LEN];
	int res;

	getcwd(c_dir,MAX_LINE_LEN-1); // save current path

	chdir(pre_path); // go to VMT folder;
	if (stat("Diagnoz.sht",&st)!=-1)
	{
		if (unlink("Diagnoz.sht")==-1)// remove previous file if exists
		{
			snprintf(msgbuf,MAX_LINE_LEN-1, "Cannot delete previous <Diagnoz.sht>: errcod=%d",errno);
			writelog(msgbuf);
			res=BAD_STATUS;
			write(0,&res,sizeof(res));
			chdir(c_dir);
			return -1;
		}
	}
//  open shared memory buffer of the collector
	hfilemapobj=shmget(VMT_SHARED_KEY + coll_data.adc_slot, sizeof(struct men_record),0/*IPC_CREAT*/ );
	if (hfilemapobj != -1)
	{
		sharedbuf = (struct men_record *)(shmat(hfilemapobj, NULL, 0));
		if (sharedbuf != (struct men_record *)(-1)) 
		{
			if (sharedbuf->diagnos==1) // *sharedbuf points to "diagnos" member
			{ // diagnose already started
				shmdt(sharedbuf);
				res=htonl(-3);
				write(0,&res,sizeof(res));
				writelog("cmd_diagnose: flag is already set !");
				chdir(c_dir);
				return -3 ;
			}
			sharedbuf->diagnos=1; // set DIAGNOSE flag
			shmdt(sharedbuf); // close file mapping and start waiting for result
			res=htonl(-100); // notify that diagnose is in progress
			write(0,&res,sizeof(res));

			tm_wait=1;
			while (1)
			{
				sleep(1); // minimize CPU load
				if  (stat("Diagnoz.sht",&st)==-1)
				{
					res=htonl(-100); // notify that diagnose is in progress
					write(0,&res,sizeof(res));
					snprintf(msgbuf,MAX_LINE_LEN-1,"Waiting for diagnose file: %d sec...",tm_wait);
					writelog(msgbuf);
				}
				else
				{
				        chdir(c_dir);
					// prepare command line for sending result
					// using getfile
					if (!cmd_args[0])
					 strcpy(cmdline,"get_file J103 Diagnoz.sht\0");
					else
           snprintf(cmdline,sizeof(cmdline),"get_file %s Diagnoz.sht\0",cmd_args[0]); 
					check_cmd_line(cmdline);
					if (!getfile(0))// send result file to client
					{
					 writelog("File <Diagnoz.sht> was sent successfully");
					 return 0;
					} 
					else
					{
					 writelog("Error sending file <Diagnoz.sht>");
					 return -1; 
					} 
				}
				tm_wait++; // increase time counter...
			} // while (1)
		}
		else // if (pSharedBuf != (int *)(-1)) 
		{
			writelog("cmd_diagnose error: shmat=-1");
			res=BAD_STATUS;  
			write(0,&res,sizeof(res));
			chdir(c_dir); 
			return -1; 
		} 
	} 
	else // 	if (hfilemapobj != -1)
	{
		writelog("cmd_diagnose error: shmget=-1");
		res=BAD_STATUS; 
		write(0,&res,sizeof(res));
		chdir(c_dir); 
		return -1;
	}

}
// Becose of UMON, SCADA_VMT, VMT_ADMIN are Win32 applications
// we need MathUserFunc.dll library
int get_parser_dll()
{
    /*
  if (!cmd_args[0])  
   strcpy(cmdline,"get_file J103 MathUserFunc.dll\0");
  else
   snprintf(cmdline,sizeof(cmdline),"get_file %s MathUserFunc.dll\0",cmd_args[0]); 
   */
  strcpy(cmdline,"get_file null ../VMT/MathUserFunc.dll\0"); 
	check_cmd_line(cmdline);
//	strcpy(cmd_args[1],"libMathUserFunc.so");
	getfile(1); // send file with waveform
	return 0;
}
int cmd_trend_freeze()
{
	unsigned char freezebyte;
	int res;
	int hfilemapobj;
	struct men_record *sharedbuf;

	res=-1;
	//	n_aggr:=-1;
	hfilemapobj=0;

	/*	if (makelock(FREEZE_LCK)==-1)
	{
		res=htonl(-3);
		write(0,&res,sizeof(res));
		writelog("Analysing already started !");
		return -3;
	}
	else
		writelog("Starting analyse session...");
	Flush(LogFile);
	end;
	res:=-1;
*/
	freezebyte=(unsigned char)(atoi(cmd_args[1]));
	if ((freezebyte != 0) && (freezebyte != 1) && (freezebyte != 2) &&
		(freezebyte != 4) && (freezebyte != 8) && (freezebyte != 16))
	{
		res=BAD_STATUS;
		write(0,&res,sizeof(res));
		snprintf(msgbuf,MAX_LINE_LEN,"cmd_trend_freeze: incorrect freeze byte - <%d>",freezebyte);
		writelog(msgbuf);
		return -1;
	}	
//	sprintf(msgbuf,"men_record=%d",/*48*/sizeof(struct men_record));
//	writelog(msgbuf);
	hfilemapobj=shmget(VMT_SHARED_KEY + coll_data.adc_slot,10/*sizeof(struct men_record)*/,0);
	if (hfilemapobj != -1)
	{	
		sharedbuf = shmat(hfilemapobj, NULL, 0);
		if (sharedbuf != (void *)(-1))
		{
			//sharedbuf += sizeof(char) * 6 + 9 ; // move to "tendfreeze" flag !! check
			if (((unsigned char)(sharedbuf->trendsfreeze) != 0) && (freezebyte != 0)) 
			{
				res = htonl(-3);
				writelog("Trend freeze is already set !");
				write(0,&res,sizeof(res));
				shmdt(sharedbuf);
				return -3;
			}
			else
			{
				(unsigned char)(sharedbuf->trendsfreeze)=freezebyte; // set "freeze" byte for specific trend
				snprintf(msgbuf,MAX_LINE_LEN-1,"cmd_trend_freeze: freeze flag <%d> is set!",freezebyte);
				writelog(msgbuf);
				res=0;
				write(0,&res,sizeof(res));
				shmdt(sharedbuf);
				return 0;
			}	
		}	
		else // if pSharedBuf <> nil then
		{	
            shmdt(sharedbuf);
			res=BAD_STATUS;
			write(0,&res,sizeof(res));
			writelog("cmd_trend_freeze:shmat error  !");
			return -1;
		}	
	}
	else // if hFileMapObj <> 0 then
	{	
		res=BAD_STATUS;
		write(0,&res,sizeof(res));
		writelog("cmd_trend_freeze:shmget error  !");
		return -1;
	}
}	

int cmd_delete_file()
{
	int n, res;
	char *fl_name   ; // buffer for target file path
	struct stat st;
	char *p;
// lpFileName:PChar;         // pointer to filename
// nSize:DWORD;              // size of file

// res=-1;
	fl_name = calloc(MAX_PATH,sizeof(char));
	if (strcasestr(cmd_args[0], "NULL")) // file is in "notyfier" root folder
		snprintf(fl_name,MAX_PATH-1,"../%s",cmd_args[1]);
	else
		snprintf(fl_name,MAX_PATH-1,"%s%s",pre_path,cmd_args[1]); // file in "vmt" folder   
	correct_case(fl_name);	
	/*	
	// check name of the file and correct case if needed
	if (p=(char*)strcasestr(fl_name,"dlevels"),p)
	 *p='d';
	if (p=(char*)strcasestr(fl_name,"vmt"),p)
	 *p='v';
	if (p=(char*)strcasestr(fl_name,"levels"),p)
	 *p='l';
	if (p=(char*)strcasestr(fl_name,"matrix"),p)
	 *p='m';
	if (p=(char*)strcasestr(fl_name,"bands"),p)
	 *p='B';
	if (p=(char*)strcasestr(fl_name,"appr"),p)
	 *p='A';
	 */
	// -------------------------------------------------- 
	snprintf(msgbuf,MAX_LINE_LEN-1,"cmd_delete_file: try to delete file <%s>...",fl_name);
	writelog(msgbuf);   
	if (stat(fl_name,&st)==-1)  // file does not exist, do nothing
	{
        writelog("file not found...");
		res=0;
		write(0,&res,sizeof(res));     
		free(fl_name);
		return 0;
	}
	else
	{
		n=TRY_COUNT;
		while (n > 0)
		{
			if (!unlink(fl_name))
			{
				res=0;
				snprintf(msgbuf,MAX_LINE_LEN-1,"cmd_delete_file: <%s> deleted OK!",fl_name);    
				writelog(msgbuf);
				write(0,&res,sizeof(res));     
				free(fl_name);
				return 0;
			}        
   // waiting for 
			snprintf(msgbuf,MAX_LINE_LEN-1," %d sec: cannot delete file, errno=%d",n,errno);
			writelog(msgbuf);
			n--;
			sleep(1);
		} // while
  // if we are here - delete fail
		sprintf(msgbuf," could not delete file !!");
		writelog(msgbuf);
		free(fl_name);
		res=BAD_STATUS;
		write(0,&res,sizeof(res));
		return -1;
	}
}
int get_note_settings()
{
	char *line;          // pointer to line
	int line_cnt,res;    // number of lines in "vmt_note.ini"
	FILE *fl;
 // format:
 // get_note_settings
 // wait for 1 byte (0 - OK)
 // receive "vmt_note.ini" (ReadStrings)
	line=calloc(MAX_LINE_LEN,sizeof(char));
 // ------------------------
	fl=fopen("../vmt_note.ini","r");
	if (fl) // try to open "vmt_note.ini"
	{
		line_cnt=0;
		res=0;
		write(0,&res,sizeof(res));
		while (fgets(line,sizeof(line),fl))
			line_cnt++;
		line_cnt=htonl(line_cnt);
		write(0,&line_cnt,sizeof(line_cnt));
		rewind(fl);
		while (fgets(line,sizeof(line),fl))
			write(0, line,strlen(line));
		fclose(fl);
		free(line);
		return 0;
	}
	else
	{
		writelog("get_note_settings: cannot open <vmt_note.ini>");
		res=BAD_STATUS;
		write(0,&res,sizeof(res));
		free(line);
		return -1;     
	}

	free(line);	 
}
int get_tcp_note()
{
	prepare_notification(); // activate notyfication handler
	while(note_active)
	 sleep(1); // just wait
	stop_notification(); // client disconnected
	return 0;
}
int get_coll_list()
{
 int res=0;
 int coll_cnt=0;
 int fl_len;
 unsigned char namelen;
 FILE *F_ini,*F_tmp;
 char buf[MAX_LINE_LEN];
 char *p;
 /*
 write(0,&res,sizeof(res)); // send OK
 res=htonl(1);// we have only 1 collector
 write(0,&res,sizeof(res));
 sprintf(msgbuf,"%s\r\n\0",coll_name);
 write(0,msgbuf,strlen(msgbuf));
 return 0;
 */
 F_ini=fopen("../vmt_note.ini","r");
 if (F_ini)
 {
  // search [Collectors] section  
  while (fgets(buf,MAX_LINE_LEN,F_ini))
  {
   if (strcasestr(buf,"[Collectors]"))    
    break;     
  }       
  if (!feof(F_ini))
  {
    F_tmp=tmpfile(); 
    if (F_tmp)
    {
    while (fgets(buf,MAX_LINE_LEN,F_ini))
 
    {  
      if (buf[0]=='[') // new section begins, exit
       break;
      p = buf; 
      if (*p != '\n') // skip empty lines
      {
       p=strtok(buf,"=");
       strcat(p,"\r\n\0");
       fwrite(p,1,strlen(p),F_tmp);
       coll_cnt++;
      } 
     }  
     snprintf(msgbuf, MAX_LINE_LEN-1, "get_coll_list: found %d collectors",coll_cnt);
     writelog(msgbuf);
     // sending number of collectors
     res=0;
     write(0, &res, sizeof(res));  //send OK
     coll_cnt = htonl(coll_cnt);
     write(0, &coll_cnt, sizeof(coll_cnt));  // count of lines 
     // sending collector list
     fl_len = ftell(F_tmp); // get length of temp file
     rewind(F_tmp);
       // move to begin of temp file
     fread(buf,1,fl_len,F_tmp); // read temp file to buffer and send
     buf[fl_len]='\0';
     write(0,buf,fl_len);
     fclose(F_tmp);
    }
    else
    {
      fclose(F_ini);
      writelog("Cannot create temp file !");
      res = BAD_STATUS;
      write(0, &res, sizeof(res));
      return -1;
    }
  } // if (!eof(F_ini))   
  else
  {
      fclose(F_ini);
      writelog("Section [Collectors] not found");
      res = BAD_STATUS;
      write(0, &res, sizeof(res));
      return -1;
  }
  // all done
  fclose(F_ini);
  return 0;
 } // if (F_ini)
 else
 {
     res = BAD_STATUS;
     write(0, &res, sizeof(res));
     writelog("Cannot open <vmt_note.ini>");
     return -1;
 }
}

// получение серийного номера платы
// Команда
// get_board_serial <vmt_name>
//  ответ:
//   0 - все OK
//   -1 - какая-то ошибка (см "vmt_note.log")
//   -2 - неверное имя сборщика
//   -3 - сборщик не запущен (т.е. его SharedMem еще не создан)
//   если ответ = 0 , то идет строка с серийником
//   читать через tcp_client.ReadLn
int get_board_serial()
{
 int hfilemapobj;
 struct men_record *sharedbuf;
 char *p;
 int res;

  //
 sharedbuf = NULL;
 hfilemapobj = 0;
  // ------------- check if VMT id running --------------------
    if (servicegetstatus("vmt") == SVC_STOPPED)
    {
       res = htonl(-3); 
	   write(0,&res,sizeof(res));  // stopped
	   writelog("get_board_serial: VMT stopped !");
       return -3;
    }
  // ---------------------------------------------------------------------
	hfilemapobj=shmget(VMT_SHARED_KEY + coll_data.adc_slot,sizeof(struct men_record),0);
	if (hfilemapobj != -1)
	{	
		sharedbuf = shmat(hfilemapobj, NULL, 0);
		if (sharedbuf != (void *)(-1))
		{
			res = 0;
			write(0,&res,sizeof(res));
			p = sharedbuf->boardserial;//p++; // skip length
			write(0,p,9); // send boardserial (String[8] = 9 bytes)
            write(0,"\n",1); 
            shmdt(sharedbuf);
            return 0;
		}	
		else // if pSharedBuf <> nil then
		{	
			res=BAD_STATUS;
			write(0,&res,sizeof(res));
			writelog("get_board_serial:shmat error  !");
			return -1;
		}	
	}
	else // if hFileMapObj <> 0 then
	{	
		res=BAD_STATUS;
		write(0,&res,sizeof(res));
		writelog("get_board_serial:shmget error  !");
		return -1;
	}

}

/*
// получение MD5 хэша платы
// Команда
// get_board_serial <vmt_name>
//  ответ:
//   0 - все OK
//   -1 - какая-то ошибка (см "vmt_note.log")
//   -2 - неверное имя сборщика
//   -3 - сборщик не запущен (т.е. его SharedMem еще не создан)
//   если ответ = 0 , то идет строка ,буфер на 32 байта
//   читать через tcp_client.Read
int get_sys_ident()
{
 int hfilemapobj;    // handle for file mapping object
 struct men_record *sharedbuf;     // pointer to file mapping buffer
 char sysidentbuffer[32];
 int res;

 sharedbuf = NULL;
 hfilemapobj = 0;
  // ------------- check if VMT id running --------------------
    if (servicegetstatus("vmt") == SVC_STOPPED)
    {
       res = htonl(-3); 
	   write(0,&res,sizeof(res));  // stopped
	   writelog("get_sys_ident: VMT stopped !");
       return -3;
    }
  // ---------------------------------------------------------------------
	hfilemapobj=shmget(16 + coll_data.adc_slot,sizeof(struct men_record),0);
	if (hfilemapobj != -1)
	{	
		sharedbuf = shmat(hfilemapobj, NULL, 0);
		if (sharedbuf != (void *)(-1))
		{
			res = 0;
			write(0,&res,sizeof(res));
			write(0,sharedbuf->system_ident_buffer,32);
            shmdt(sharedbuf);
            return 0;
		}	
		else // if pSharedBuf <> nil then
		{	
			res=BAD_STATUS;
			write(0,&res,sizeof(res));
			writelog("get_sys_ident:shmat error  !");
			return -1;
		}	
	}
	else // if hFileMapObj <> 0 then
	{	
		res=BAD_STATUS;
		write(0,&res,sizeof(res));
		writelog("get_sys_ident:shmget error  !");
		return -1;
	}
}
*/
/*
// EPROM version
int set_vmt_serial()
{
    int res;
    DSCB dscb;    // handle used to refer to the board
    DSCCB dsccb;
    ERRPARAMS errorParams; // structure for returning error code and error string
    char *vmtkeypath;
    char *vmtkeybuf;  // for key file
    FILE *vmtkey;
    int i;
    unsigned long keyfilelen;

   	if( dscInit( DSC_VERSION ) != DE_NONE )
	{
		dscGetLastError(&errorParams);
		sprintf( msgbuf,"set_vmt_serial: dscInit error: %s %s\n", dscGetErrorString(errorParams.ErrCode), errorParams.errstring );	
		writelog(msgbuf);
//		syslog(LOG_ERR|LOG_LOCAL0,msgbuf );
        res = BAD_STATUS;
        write(0,&res,sizeof(res));
		return -1;
	}

 
	dsccb.boardtype = DSC_HERCEBX;
	dsccb.base_address = 0x240;
	dsccb.int_level = 5;
	if(dscInitBoard(dsccb.boardtype, &dsccb, &dscb) != DE_NONE)
	{
		dscGetLastError(&errorParams);
		sprintf( msgbuf, "set_vmt_serial: dscInitBoard error: %s %s\n", dscGetErrorString(errorParams.ErrCode), errorParams.errstring );	
		writelog(msgbuf);
//		syslog(LOG_ERR|LOG_LOCAL0,msgbuf );
        res = BAD_STATUS;
        write(0,&res,sizeof(res));
		return -1;
	}
	// reading EPROM (128-255) into eprom_buf
	for (i=0;i<=127;i++)
	{
	 if (dscGetEEPROM(dscb, i, &eprom_buf[i]) != DE_NONE)
	 {
		dscGetLastError(&errorParams);
		sprintf( msgbuf, "set_vmt_serial: dscInitBoard error: %s %s\n", dscGetErrorString(errorParams.ErrCode), errorParams.errstring );	
		writelog(msgbuf);
    res = BAD_STATUS;
    dscFree();
    write(0,&res,sizeof(res));
		return -1;
   }
  }  
	 // sending EPROM 
	 res=0;
	 write(0,&res,sizeof(res));
	 write(0,eprom_buf,128);
	 // wait for key file
	 read(0,&keyfilelen,sizeof(keyfilelen));
	 keyfilelen = ntohl(keyfilelen);
	 vmtkeybuf = calloc(keyfilelen, sizeof(char));
	 read(0,vmtkeybuf,keyfilelen);
	 // save to "vmtkey" file in vmt working folder
	 vmtkeypath = calloc(MAX_PATH, sizeof(char));
	 snprintf(vmtkeypath,MAX_PATH-1,"%s/vmtkey",ROOT_PROG_PATH);
	 unlink(vmtkeypath); // delete previous key file
	 vmtkey = fopen(vmtkeypath,"w");
	 if (vmtkey)
	 {
	  fwrite(vmtkeybuf,1,keyfilelen,vmtkey);
	  free(vmtkeybuf);
	  free(vmtkeypath);
	  fclose(vmtkey);
	  writelog("set_vmt_serial: file 'vmtkey' created successfully !");     
   }
   else
   { 
       writelog("set_vmt_serial: cannot create 'vmtkey' file !");     
       free(vmtkeybuf);
	   free(vmtkeypath);
       res = BAD_STATUS;
       write(0,&res,sizeof(res));
       dscFree();
	   return -1;	 
   }
	 dscFree();
	 res=0;
	 write(0,&res,sizeof(res));
	 return 0;
}
*/
// identify version
int set_vmt_serial()
{
    int res;
    char *vmtkeypath;
    char *vmtkeybuf;  // for key file
    char *ident_buf;
    FILE *ident,*vmtkey;
    int i;
    unsigned long keyfilelen,identlen;
    unsigned long identlen_net;
    struct stat st;
    FILE *f_tmp;
    int bytes_read;

     // reading "identify"
     ident = fopen(IDENTIFY_PATH,"r");
     if (!ident)
     {
       writelog("set_vmt_serial: cannot get system data (fopen)!");                    
       res = BAD_STATUS;
       write(0,&res,sizeof(res));
       return -1;	 
     }
     ident_buf = calloc(128, sizeof(char));
     f_tmp = tmpfile();
     while (!feof(ident))
     {
      bytes_read = fread(ident_buf,1,128,ident);
      fwrite(ident_buf,1,bytes_read,f_tmp);
     }
     fclose(ident);
     free(ident_buf);
     identlen=ftell(f_tmp);
     ident_buf = calloc(identlen + 1,sizeof(char));
     rewind(f_tmp); 
     if (!fread(ident_buf,1,identlen,f_tmp))
     {
       fclose(f_tmp);
       free(ident_buf);                                 
       writelog("set_vmt_serial: cannot get system data (fread)!");
       res = BAD_STATUS;
       write(0,&res,sizeof(res));
       return -1;	 
     }
         fclose(f_tmp);
	 // sending "identify" 
	 res=0;
	 write(0,&res,sizeof(res));
	 identlen_net = htonl(identlen);
	 write(0,&identlen_net,sizeof(identlen_net));
	 write(0,ident_buf,identlen);
	 free(ident_buf);
	 // wait for key file
	 read(0,&keyfilelen,sizeof(keyfilelen));
	 keyfilelen = ntohl(keyfilelen);
	 vmtkeybuf = calloc(keyfilelen, sizeof(char));
	 read(0,vmtkeybuf,keyfilelen);
	 // save to "vmtkey" file in vmt working folder
	 vmtkeypath = calloc(MAX_PATH, sizeof(char));
	 snprintf(vmtkeypath,MAX_PATH-1,"%s/vmtkey",ROOT_PROG_PATH);
	 unlink(vmtkeypath); // delete previous key file
	 vmtkey = fopen(vmtkeypath,"w");
	 if (vmtkey)
	 {
	  fwrite(vmtkeybuf,1,keyfilelen,vmtkey);
	  free(vmtkeybuf);
	  free(vmtkeypath);
	  fclose(vmtkey);
	  writelog("set_vmt_serial: file 'vmtkey' created successfully !");     
   }
   else
   { 
       writelog("set_vmt_serial: cannot create 'vmtkey' file !");     
       free(vmtkeybuf);
	   free(vmtkeypath);
       res = BAD_STATUS;
       write(0,&res,sizeof(res));
//       dscFree();
	   return -1;	 
   }
//	 dscFree();
	 res=0;
	 write(0,&res,sizeof(res));
	 return 0;
}

int get_vmt_serial()
{
    struct stat buf_st;
    char *vmtkeypath;
    char *vmtkeybuf;  // for key file
    int res;
    int keyfilelen;
    FILE *vmtkey;
    
   	 vmtkeypath = calloc(MAX_PATH, sizeof(char));
	 snprintf(vmtkeypath,MAX_PATH-1,"%s/vmtkey",ROOT_PROG_PATH);
	 if (stat(vmtkeypath,&buf_st) != -1)
	 {
      vmtkeybuf=calloc(buf_st.st_size + 1,sizeof(char));      
	  vmtkey = fopen(vmtkeypath,"r");
	  if (vmtkey)
	  {
	   fread(vmtkeybuf,1,buf_st.st_size,vmtkey);
	   res=0;
	   write(0,&res,sizeof(res));
	   keyfilelen = htonl(buf_st.st_size);
	   write(0,&keyfilelen,sizeof(keyfilelen));
	   write(0,vmtkeybuf,buf_st.st_size);
	   free(vmtkeybuf);
	   free(vmtkeypath);
	   fclose(vmtkey);
	   writelog("set_vmt_serial: file 'vmtkey' sent successfully !");     
	   got_vmt_key = 1;
	   return 0;
    }
    else
    {
       writelog("set_vmt_serial: cannot open 'vmtkey' file !");     
       free(vmtkeybuf);
	   free(vmtkeypath);
       res = BAD_STATUS;
       write(0,&res,sizeof(res));
	   return -1;	 
    }
    }  
    else
    {
       writelog("set_vmt_serial: 'vmtkey' not found !");     
       free(vmtkeypath);
       res = BAD_STATUS;
       write(0,&res,sizeof(res));
	   return -1;	 
    } 

}
