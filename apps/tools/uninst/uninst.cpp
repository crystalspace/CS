/*
    Copyright (C) 2000 by W.C.A. Wijngaards

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


    Description:
	Portable uninstall program.
	(will delete a set of files, and resulting empty directories)

	It will either read install.log in the working directory, or
	read install.log at the pathname given as argument.

	install.log contains the pathnames of the files that are installed
	(and will be deleted) seperated by whitespace. Duplicates will be
	removed.

	(include install.log and uninst.exe pathnames into install.log
	if you wish them to be deleted as well.)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// global information
int nr_files = 0;
struct file_info {
  char *name;
  int deleted;
  struct file_info* next;
} *files = 0;

int nr_dirs = 0;
struct file_info *dirs = 0;


// a strdup (for portability to e.g. NeXT)
char *mystrdup(char *orig)
{
  int len = strlen(orig);
  char *dest = (char*)malloc(len+1);
  memcpy(dest, orig, len+1); // also copy terminating 0
  return dest;
}

// extract the notdir part of a pathname, returns a ptr in the same string
char *notdir(char *str)
{
  int len = strlen(str);
  int i= len-1;
  while(i>=0 && str[i]!='/' && str[i]!='\\' && str[i]!=':')
    i--;
  return &(str[i+1]);  /// possibly a "" string.
}


// get directory part of a pathname (assumes absolute pathnames)
void getdirpart(char *dest, char *src)
{
  char *cp = src;
  int i=0;
  //int len = strlen(src);
  char *ndir = notdir(src);

  while(cp < ndir)
  {
    dest[i++] = *(cp++);
  }
  dest[i] = 0;
  // remove trailing / \ or :
  if(i>0) dest[i-1] = 0;
}


// returns true if a file name exists in the list
int file_exists(struct file_info *start, char *checkname)
{
  struct file_info* fp= start;
  while(fp)
  {
    if(strcmp(fp->name, checkname)==0)
      return 1;
    fp = fp->next;
  }
  return 0;
}


// read all files from install.log, removing duplicates
void readallfiles(FILE *in)
{
  nr_files = 0;
  files = 0;
  char buf[500];
  struct file_info *newfile = 0;

  while( fscanf(in, " %499s", buf) == 1)
  {
    // duplicate ?
    if(file_exists(files, buf)) continue;
    // add
    nr_files ++;
    newfile = (struct file_info*)malloc(sizeof(struct file_info));
    newfile->name = mystrdup(buf);
    newfile->deleted = 0;
    newfile->next = files;
    files = newfile;
    //printf("Added file %s\n", buf);
  }
}


// get dirs from file list
void getdirs(void)
{
  nr_dirs = 0;
  dirs = 0;
  struct file_info *fp = files;
  struct file_info *newdir=0;
  char buf[500];

  while(fp)
  {
    // get directory part of a file
    getdirpart(buf, fp->name);
    // duplicate?
    if(file_exists(dirs, buf))
    {
      fp = fp->next;
      continue;
    }
    // add
    nr_dirs ++;
    newdir = (struct file_info*)malloc(sizeof(struct file_info));
    newdir->name = mystrdup(buf);
    newdir->deleted = 0;
    newdir->next = dirs;
    dirs = newdir;

    //printf("Added dir %s\n", buf);

    fp = fp->next;
  }
}


/// delete all files.
void delete_files()
{
  struct file_info *fp = files;

  while(fp)
  {
    /// do not complain if it fails. (returns -1)
    /// but if it works (returns 0) set deleted to true
    if(remove(fp->name) == 0)
      fp->deleted = 1;
    fp = fp->next;
  }
}


/// returns true if a dir could be deleted
int try_delete_dirs()
{
  struct file_info *dp = dirs;
  int succeed = 0;

  while(dp)
  {
    /// try deleting it.
    /// either use   int rmdir(char*) from unistd, or
    ///        use   int remove(char*) from stdio.
    /// both return -1 on failure, 0 on success.
    /// (and for both the directory must be empty on Linux)
    if( remove(dp->name) == 0 ) 
    {
      dp->deleted = 1;
      succeed = 1;
    }

    dp = dp->next;
  }
  return succeed;
}


/// delete only empty directories, but try deleting subdirs first.
void delete_dirs()
{
  /// try deleting all dirs until no dir can be deleted any more...
  while(try_delete_dirs())
    ;
}


/// clean up memory
void destruct()
{
  struct file_info *fp = files;
  struct file_info *nextfp;
  int all_files_deleted = 1;
  int all_dirs_deleted = 1;
  while(fp)
  {
    nextfp = fp->next;
    if(!fp->deleted)
    {
      all_files_deleted = 0;
      //printf("File %s could not be deleted.\n", fp->name);
    }
    free(fp->name);
    free(fp);
    fp = nextfp;
  }
  nr_files = 0;
  files = 0;

  fp = dirs;
  while(fp)
  {
    nextfp = fp->next;
    if(!fp->deleted)
    {
      all_dirs_deleted = 0;
      //printf("Directory %s could not be deleted.\n", fp->name);
    }
    free(fp->name);
    free(fp);
    fp = nextfp;
  }
  nr_dirs = 0;
  dirs = 0;

  if(!all_dirs_deleted)
  {
    printf("\nSome directories could not be deleted. These could contain\n"
      "temporary files or user data. Investigate by hand.\n\n");
  }
}


int main(int argc, char *argv[])
{
  char *install_log;
  char *notdir_inst;
  FILE *in;

  if( argc < 1 || argc > 2 )
  {
    printf("Usage: %s install.log\n", argv[0]);
    exit(1);
  }

  // default to current dir. (i.e. double click ?!)
  if(argc == 2)
    install_log = argv[1];
  else install_log = "install.log";
  /// get the notdir part.
  notdir_inst = notdir(install_log);
  if(strcmp(notdir_inst, "install.log") != 0)
  {
    printf("This program will only uninstall from a file 'install.log'.\n");
    exit(1);
  }

  /// read all from install.log
  in=fopen(install_log, "r");
  if(in == 0) {
    printf("No uninstallation possible: could not open %s\n", install_log);
    perror(argv[0]);
    exit(1);
  }
  readallfiles(in);
  getdirs();
  fclose(in);

  /// delete files
  delete_files();

  /// delete empty directories
  delete_dirs();

  /// clean up memory
  destruct();

  return 0;
}

