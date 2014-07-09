#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define VERSION "0.1"

void usage() {
  printf("Usage: fixperms [-f 644] [-d 755] <directory>|<-h>\n");
  exit(1);
}

void help() {
  printf("Usage: fixperms [-f 644] [-d 755] <directory>|<-h>\n");
  printf("Sets the permission of every file in \"directory\" to the\n");
  printf("permissions specified by \"-f\" (default: 644). Same applies\n");
  printf("to directories, but the permission specified by \"-d\".\n");
  printf("Special files (links, sockets etc.) will be skipped.\n");
  printf("fixperms version "VERSION"\n");
  exit(0);
}

int octalMode(char* octals, mode_t* mode) {
  if( strlen(octals) != 3 )
    return -1;

  char octal = octals[0]-'0';
  if( octal < 0 || octal > 7 )
    return -2;
  if( octal & 4 )
    *mode |= S_IRUSR;
  else
    *mode &= ~S_IRUSR;
  if( octal & 2 )
    *mode |= S_IWUSR;
  else
    *mode &= ~S_IWUSR;
  if( octal & 1 )
    *mode |= S_IXUSR;
  else
    *mode &= ~S_IXUSR;

  octal = octals[1]-'0';
  if( octal < 0 || octal > 7 )
    return -2;
  if( octal & 4 )
    *mode |= S_IRGRP;
  else
    *mode &= ~S_IRGRP;
  if( octal & 2 )
    *mode |= S_IWGRP;
  else
    *mode &= ~S_IWGRP;
  if( octal & 1 )
    *mode |= S_IXGRP;
  else
    *mode &= ~S_IXGRP;

  octal = octals[2]-'0';
  if( octal < 0 || octal > 7 )
    return -2;
  if( octal & 4 )
    *mode |= S_IROTH;
  else
    *mode &= ~S_IROTH;
  if( octal & 2 )
    *mode |= S_IWOTH;
  else
    *mode &= ~S_IWOTH;
  if( octal & 1 )
    *mode |= S_IXOTH;
  else
    *mode &= ~S_IXOTH;

  return 0;
}

int fixperms(char* path, mode_t filemode, mode_t dirmode) {
  DIR* dir;
  struct dirent* direntry;
  struct stat dirstat;

  dir = opendir(path);
  if( dir == NULL ) {
    fprintf(stderr, "Failed to open directory %s\n", path);
    return -1;
  }

  while( (direntry = readdir(dir)) ) {
    if( !strcmp(direntry->d_name, "..") || !strcmp(direntry->d_name, ".") )
      continue;
    char* epath = malloc(strlen(path)+strlen(direntry->d_name)+2);
    strcpy(epath, path);
    strcat(epath, "/");
    strcat(epath, direntry->d_name);
    if( stat(epath, &dirstat) ) {
      fprintf(stderr, "Failed to stat %s\n", epath);
      continue;
    }

    if( S_ISDIR(dirstat.st_mode) ) {
      fixperms(epath, filemode, dirmode);
      if( dirstat.st_mode != (dirmode | S_IFDIR) )
	if( chmod(epath, dirmode) )
	  fprintf(stderr, "Failed to update the permissions of directory %s\n", epath);
    } else {
      if( S_ISREG(dirstat.st_mode) ) {
	if( dirstat.st_mode != (filemode | S_IFREG) )
	  if( chmod(epath, filemode) )
	    fprintf(stderr, "Failed to update the permissions of file %s\n", epath);
      } else
	printf("Ignored special file %s\n", epath);
    }

    free(epath);
  }

  closedir(dir);

  return 0;
}

int main(int argc, char** argv) {
  mode_t filemode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  mode_t dirmode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
  char* directory = 0;

  if( argc < 2 )
    usage();
  argc--;

  while( argc ) {
    argv++;
    argc--;

    //Parse "-h" parameter
    if( !strcmp(*argv,"-h") )
      help();

    //Parse "-f" parameter
    if( !strcmp(*argv,"-f") ) {
      if( !argc )
	usage();
      argv++;
      argc--;
      if( octalMode(*argv, &filemode) ) {
	fprintf(stderr,"Invalid mode: %s\n",*argv);
	usage();
      }
      continue;
    }

    //Parse "-d" parameter
    if( !strcmp(*argv,"-d") ) {
      if( !argc )
	usage();
      argv++;
      argc--;
      if( octalMode(*argv, &dirmode) ) {
	fprintf(stderr,"Invalid mode: %s\n",*argv);
	usage();
      }
      continue;
    }

    //Parse directory
    if( directory != 0 ) { //set directory only once
      fprintf(stderr,"Unknown parameter: %s\n",*argv);
      usage();
    }
    directory = malloc(strlen(*argv)+1);
    strcpy(directory,*argv);
  }

  fixperms(directory, filemode, dirmode);
  free(directory);

  return 0;
}
