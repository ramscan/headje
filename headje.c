/*
 * Jim's Head -f
 *
 * (c) 2019 James Goodmon
 *
 * 2019 03 23 Initial construction
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <string.h>		// memcpy
#include <libgen.h>		// basename

#include <fcntl.h>
#include <termios.h>	// tcsetattr,TCSANOW
#include <sysexits.h>	// EX_OK
#include <unistd.h>		// stat

#include <rpc/rpc.h>	// added so VSCode will SHUT UP! about fd_set

#include <sys/types.h>	// stat
#include <sys/stat.h>	// S_IRxxx

#include <sys/ioctl.h>	// ioctl

#define BUFSIZE	16726

struct winsize wsz;
struct termios opts;
struct termios nopts;

char *buf;
char RUN;

char bfname[80];

// may get a character from stdin, may not
int mayget(int secs)
{
	int c;
	struct timeval tv;
	fd_set	rdfs;

	c=0;

	tv.tv_sec=secs; tv.tv_usec=0;		// this determins the interval between updates

	FD_ZERO(&rdfs);
	FD_SET(STDIN_FILENO,&rdfs);

	select(STDIN_FILENO+1,&rdfs,NULL,NULL,&tv);
	if ( FD_ISSET(STDIN_FILENO,&rdfs) ) c=getchar();

	return(c);
}

// get the window size, in characters
void get_wsz(void)
{
	ioctl(0,TIOCGWINSZ, &wsz);
}

// null terminate when the first cr or lf is found
// this is needed because puts will automatically add an lf
void clipcr(char *s)
{
	while( *s ) {
		if ( *s=='\n' ) { *s=0x00; return; }
		if ( *s=='\r' ) { *s=0x00; return; }
		++s;
	}
}

// read the first n lines of a text file
void readhead(char *fn,int lines)
{
	FILE *file;
	struct stat st;
	int err;
	int l,w;
	struct tm *ct;
	time_t t;
	char *p;

	err=stat(fn,&st);
	if ( err<0 ) { fprintf(stdout,"Could not stat \"%s\"\n",fn); return; }
	t=st.st_mtime;
	ct=localtime(&t); strftime(buf,BUFSIZE,"%T",ct);

	l=0; w=(wsz.ws_col)-1;
	if ( (file=fopen(fn,"r"))==NULL ) {
		fprintf(stdout,"Could not open \"%s\"\n",fn);
		return;
	}

	fprintf(stdout,"\33[1;1H\33[J");
	if ( err>=0 ) fprintf(stdout,"%s ",buf);
	puts(bfname);

	for(l=0; l<lines; ++l) {
		p=fgets(buf,BUFSIZE,file);
		if ( p ) {
			clipcr(buf); buf[w]=0;
			puts(buf);
		}
		if ( feof(file) ) l=lines;
	}
	fclose(file);
}

int main(int argc,char **argv)
{
	struct stat st;
	int argn;
	int lines,lrow,lcol;
	int err;
	int z;
	char *fnp;

//	char ESC,SEQ;

	time_t mt,lmt;

	argn=1;
	buf=NULL;
	RUN=1;

//	ESC=0; SEQ=0;

	if ( !(buf=malloc(BUFSIZE)) ) {
		puts("no buffer memory");
		return(EX_OK);
	}

	setvbuf(stdin ,NULL,_IONBF,0);	// unbuffer input
	setvbuf(stdout,NULL,_IONBF,0);	// unbuffer output

	tcgetattr(0,&opts);				// save the current tty opts
	memcpy(&nopts,&opts,sizeof(struct termios));

	nopts.c_lflag    &= ~(ICANON | ECHO | IEXTEN | ISIG );
	nopts.c_iflag    &= ~(IXON);	// ignore ctrl-s ctrl-q
	nopts.c_cc[VMIN]  = 1;
	nopts.c_cc[VTIME] = 0;

	tcsetattr(0,TCSANOW,&nopts);	// set new tty opts

	fnp=argv[argn];
	strncpy(bfname,basename(fnp),78); bfname[79]=0;

	mt=0; lmt=0;

	get_wsz();
	lrow=wsz.ws_row; lcol=wsz.ws_col;
	lines=lrow-1;

	readhead(fnp,lines);

	while( RUN ) {
		z=mayget(2);

		if ( z==-1  ) RUN=0;
		if ( z== 3  ) RUN=0;
		if ( z=='q' ) RUN=0;
/*
		if ( z=='A' && SEQ ) {} // Up
		if ( z=='B' && SEQ ) {} // Dn

		if ( SEQ ) SEQ=0;

		if ( z==27 ) ESC=1;
		if ( z=='[' && ESC ) { SEQ=1; ESC=0; }
*/
		if ( RUN ) {
			get_wsz();

			err=stat(fnp,&st);
			if ( err>=0 ) mt=st.st_mtime;

			if ( mt!=lmt || wsz.ws_row!=lrow || wsz.ws_col!=lcol ) {
				lrow=wsz.ws_row; lcol=wsz.ws_col;
				lines=lrow-1;
				--lines;
				readhead(fnp,lines);

				lmt=mt;
			}
		}
	}

	fprintf(stdout,"\33[%d;1H\33[KStopped\n",wsz.ws_row-1);

	if ( buf ) { free(buf); buf=NULL; }

	tcsetattr(0,TCSANOW,&opts);		// restore tty opts

	return(EX_OK);
}
