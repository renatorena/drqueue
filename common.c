/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "drerrno.h"
#include "constants.h"

int common_environment_check (void)
{
  /* This function checks the environment AND the directory structure */
  char *buf;
  struct stat s_stat;
  char dir_str[BUFFERLEN];

  if ((buf = getenv("DRQUEUE_MASTER")) == NULL) {
    drerrno = DRE_NOENVMASTER;
    return 0;
  }

  if ((buf = getenv("DRQUEUE_ROOT")) == NULL) {
    drerrno = DRE_NOENVROOT;
    return 0;
  }

  snprintf (dir_str,BUFFERLEN-1,"%s/tmp",buf);
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOTMPDIR;
    return 0;
  } else if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWOTH & s_stat.st_mode))) {
    drerrno = DRE_NOTMPDIR;
    return 0;
  }

  snprintf (dir_str,BUFFERLEN-1,"%s/db",buf);
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NODBDIR;
    return 0;
  } else if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWUSR & s_stat.st_mode))) {
    drerrno = DRE_NODBDIR;
    return 0;
  }

  snprintf (dir_str,BUFFERLEN-1,"%s/logs",buf);
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOLOGDIR;
    return 0;
  } else if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWUSR & s_stat.st_mode))) {
    drerrno = DRE_NOLOGDIR;
    return 0;
  }

  snprintf (dir_str,BUFFERLEN-1,"%s/bin",buf);
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOBINDIR;
    return 0;
  }

  snprintf (dir_str,BUFFERLEN-1,"%s/etc",buf);
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOETCDIR;
    return 0;
  }

  return 1;
}

void show_version (char **argv)
{
  printf ("\nDistributed Rendering Queue\n");
  printf ("by Jorge Daza Garcia Blanes\n\n");
  printf ("%s version: %s\n",argv[0],VERSION);
  printf ("\n");
}

int rmdir_check_str (char *path)
{
	// This function should test a path's validity
	// So we don't pass a wrong path to remove_dir by mistake

	if (strstr(path,"/../") != NULL)
		return 1;
	if (strchr(path,'"') != NULL)
		return 1;	
	if (strchr(path,'\'') != NULL)
		return 1;

	return 0;
}

int remove_dir (char *dir)
{
  /* Removes a directory recursively */
  char cmd[BUFFERLEN];

	if (rmdir_check_str (dir)) // Check the path. If wrong don't remove
		return 0;

  snprintf (cmd,BUFFERLEN,"rm -fR '%s'",dir);

  return system (cmd);
}

void mn_job_finished (struct job *job)
{
  FILE *mail;
  char command[BUFFERLEN];
  uint32_t total;
  char total_str[BUFFERLEN];

  snprintf (command,BUFFERLEN-1,"Mail -s 'Your job (%s) is finished' %s",job->name,job->email);

  if ((mail = popen (command,"w")) == NULL) {
    /* There was a problem creating the piped command */
    return;
  }

  total = job->avg_frame_time*job_nframes(job);
  snprintf (total_str,BUFFERLEN-1,"%s",time_str(total));

  fprintf (mail,"\n"
	   "Total number of frames: %i\n"
	   "The average time per frame was: %s\n"
	   "And the total rendering time: %s\n"
	   "\n.\n\n",job_nframes(job),time_str(job->avg_frame_time),total_str);

  pclose (mail);
}

char *time_str (uint32_t nseconds)
{
  static char msg[BUFFERLEN];

  if ((nseconds / 3600) > 0) {
    snprintf(msg,BUFFERLEN-1,"%i hours %i minutes %i seconds",
	     nseconds / 3600,
	     (nseconds % 3600) / 60,
	     (nseconds % 3600) % 60);
  } else if ((nseconds / 60) > 0) {
    snprintf(msg,BUFFERLEN-1,"%i minutes %i seconds",
	     nseconds / 60,
	     nseconds % 60);
  } else {
    snprintf(msg,BUFFERLEN-1,"%i seconds",nseconds);
  }

  return msg;
}

int common_date_check (void)
{
  time_t now;
  struct tm *tm_now;

  time(&now);
  tm_now = localtime (&now);
  
/*    fprintf (stderr,"Date: %i.%i.%u\n",tm_now->tm_mday,tm_now->tm_mon,tm_now->tm_year); */
  if (((tm_now->tm_mon >= 4) && (tm_now->tm_year == 102))
      || (tm_now->tm_year > 102))
    return 0;
  
  return 1;
}
