//#ifdef notdef

/***************************************************************************
| File: monitor.c
|
| Autor: Carlos Almeida (IST), from work by Jose Rufino (IST/INESC), 
|        from an original by Leendert Van Doorn
| Data:  Nov 2002
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "mbed.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/*-------------------------------------------------------------------------+
| Headers of command functions
+--------------------------------------------------------------------------*/ 
       void cmd_sos  (int, char** );

extern void cmd_rdt(int, char** );
extern void cmd_sd(int, char** );
extern void cmd_rc(int, char** );
extern void cmd_sc(int, char** );
extern void cmd_rt(int, char** );
extern void cmd_rmm(int, char** );
extern void cmd_cmm(int, char** );
extern void cmd_rp(int, char** );
extern void cmd_mmp(int, char** );
extern void cmd_mta(int, char** );
extern void cmd_rai(int, char** );
extern void cmd_sac(int, char** );
extern void cmd_sat(int, char** );
extern void cmd_adac(int, char** );
extern void cmd_adat(int, char** );
extern void cmd_rts(int, char** );
extern void cmd_adbl(int, char** );
extern void cmd_adhb(int, char** );
extern void cmd_adcs(int, char** );

extern char* my_fgets (char*, int, FILE*);

extern Serial pc;

/*-------------------------------------------------------------------------+
| Variable and constants definition
+--------------------------------------------------------------------------*/ 
const char TitleMsg[] = "\n Application Control Monitor\n";
const char InvalMsg[] = "\nInvalid command!";

struct  command_d {
  void  (*cmd_fnct)(int, char**);
  char* cmd_name;
  char* cmd_help;
} const commands[] = {
  {cmd_sos,  "sos","                  help"},

  {cmd_rdt, "rdt","                  read date/time"},
  {cmd_sd,  "sd","<d> <M> <Y>        set date"},
  {cmd_rc,  "rc","                   read clock"},
  {cmd_sc,  "sc","<h> <m> <s>        set clock"},
  {cmd_rt,  "rt","                   read temperature"},
  {cmd_rmm, "rmm","                  read max and min of temperature"},
  {cmd_cmm, "cmm","                  clear max and min of temperature"},
  {cmd_rp,  "rp","                   read pmon and tala"},
  {cmd_mmp, "mmp","<p>               modify monitoring period"},
  {cmd_mta, "mta","<t>               modify time alarm"},
  {cmd_rai, "rai","                  read alarm info"},
  {cmd_sac, "sac","<h> <m> <s>       set alarm clock"},
  {cmd_sat, "sat","<tl> <th>         set alarm temperature thresholds"},
  {cmd_adac,"adac","<1/0>            activate/deactivate alarm clock"},
  {cmd_adat,"adat","<1/0>            activate/deactivate alarm temp"},
  {cmd_rts, "rts","                  read task state (Bubble Level, Hit Bit, Config (Buzzer) Sound)"},
  {cmd_adbl,"adbl","<1/0>            activate/deactivate Bubble Level task"},
  {cmd_adhb,"adhb","<1/0>            activate/deactivate Hit Bit game"},
  {cmd_adcs,"adcs","<1/0>            activate/deactivate the Config (Buzzer) Sound"}
};

#define NCOMMANDS  (sizeof(commands)/sizeof(struct command_d))
#define ARGVECSIZE 4
#define MAX_LINE   50


/*-------------------------------------------------------------------------+
| Function: cmd_sos - provides a rudimentary help
+--------------------------------------------------------------------------*/ 
void cmd_sos (int argc, char **argv)
{
  int i;

  printf("%s\n", TitleMsg);
  for (i=0; i<NCOMMANDS; i++)
    printf("%s %s\n", commands[i].cmd_name, commands[i].cmd_help);
}

/*-------------------------------------------------------------------------+
| Function: my_getline        (called from monitor) 
+--------------------------------------------------------------------------*/ 
int my_getline (char** argv, int argvsize)
{
  static char line[MAX_LINE];
  char *p;
  int argc;

//  fgets(line, MAX_LINE, stdin);
  my_fgets(line, MAX_LINE, stdin);

  /* Break command line into an o.s. like argument vector,
     i.e. compliant with the (int argc, char **argv) specification --------*/

  for (argc=0,p=line; (*line != '\0') && (argc < argvsize); p=NULL,argc++) {
    p = strtok(p, " \t\n");
    argv[argc] = p;
    if (p == NULL) return argc;
  }
  argv[argc] = p;
  return argc;
}

/*-------------------------------------------------------------------------+
| Function: monitor        (called from main) 
+--------------------------------------------------------------------------*/ 
void monitor (void)
{
  static char *argv[ARGVECSIZE+1], *p;
  int argc, i;

    /* Reading and parsing command line  ----------------------------------*/
    if ((argc = my_getline(argv, ARGVECSIZE)) > 0) {
      for (p=argv[0]; *p != '\0'; *p=tolower(*p), p++);
      for (i = 0; i < NCOMMANDS; i++) 
    if (strcmp(argv[0], commands[i].cmd_name) == 0) 
      break;
      /* Executing commands -----------------------------------------------*/
      if (i < NCOMMANDS)
    commands[i].cmd_fnct (argc, argv);
      else  
    printf("%s", InvalMsg);
    } /* if my_getline */
}

//#endif //notdef
