#ifndef DNOTES_TCP_H
#define DNOTES_TCP_H
#include <fcntl.h>	/* in glibc 2.2 this has the needed
				   values defined */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define VMT_NOTE_KEY 0x20
#define VMT_NOTE_LEN 4096

int note_active;

//void sendnote();
/*static*/ void handler(int sig, siginfo_t *sig_info, void *sig_context);
void prepare_notification();
void stop_notification();
#endif
