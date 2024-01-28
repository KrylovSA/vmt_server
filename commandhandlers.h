#ifndef COMMANDHANDLERS_H
#define COMMANDHANDLERS_H

#include <time.h>
#define MAX_CMD_LINE 128
#define MAX_CMD_ARGS 10
#define MAX_LOGS 100  // max number of log records (files) 
#define ANALYSE_LCK "/tmp/analyse.lck"
#define ANALYSE_TMOUT 30
#define ANALYSE
#define TRY_COUNT   5 // times to try delete file in cmd_delete_file
#define BAD_STATUS htonl(-1)

#define ROOT_PROG_PATH "/usr/local/Hercules/vmt" // path to vmt

//#define LOCK_GUEST  "/tmp/lock_guest"
//#define LOCK_TMOUT  60
//#define CLNLIST_FILE  "../Clients.dat"
#define VMT_SHARED_KEY 0x10
#define IDENTIFY_PATH "/proc/ide/ide0/hda/identify"
// struct for storing log files
struct logrec
{
	char fname[16]; // name of log file
	time_t ftime;   // date/time
//	struct logrec *next; // pointer to next struct (linked list)
//	struct logrec *prev; // pointer to previous linked list)
};
struct men_record
{
//	pMenRecord = ^TMenRecord;
	char  diagnos ;  // 1, when diagnose mode is ON
	char  monitor ;
	char  vmton   ;
	char	terminate ;    // 1, when we want to stop vmt.exe (application mode in Win32 version)
	char	vmtvisible ;
	char	boardserial[9]; // string[8] in Delphi, so 1 byte length + 8 byte data
	char  dofft;
	unsigned char trendsfreeze; // 
//	char system_ident_buffer[32];// buffer for illegal copy protection 
};

char cmdline[MAX_CMD_LINE];
char *cmd_args[MAX_CMD_ARGS];
char *pre_path;

int check_cmd_line(char *cmdline);
void do_command(char *cmdline, char *cmd_args[]);
void tmstamp(char *s);
int getfile(int need_pack);
int get_last_file();
int get_coll_settings();
int cmd_status();
int cmd_start();
int cmd_stop();
int get_trend_real();
int get_trend_min();
int get_trend_hour();
int get_trend_upl();
int get_rpm_trend();
int get_diag_trend();
int get_time();
int cmd_send_file();
int get_log_list();
int get_log_file();
int cmd_analyse();
int cmd_diagnose();
int get_parser_dll();
int cmd_trend_freeze();
int cmd_delete_file();
int get_note_settings();
int get_tcp_note();
int get_board_serial();
//int get_sys_ident();
int set_vmt_serial();
int get_vmt_serial();
int fn_logcmp(const void *p, const void *p1);
#endif
