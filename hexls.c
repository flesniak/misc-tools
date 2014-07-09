#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

void printHex(unsigned char* string, int len) {
  if( len == 0 )
    return;
  int i = 0;
  while( string[i] != 0 ) {
    printf("%x ", string[i]);
    i++;
    if( i == len )
      break;
  }
}

int main(int argc, char** argv) {
  if( argc > 2 ) {
    printf("Usage: ls [directory]\n");
    exit(1);
  }

  DIR* dir;
  struct dirent* direntry;
  char* path;

  if( argc == 1 ) {
    path = malloc(2);
    strcpy(path, ".");
  } else {
    argv++;
    path = malloc(strlen(*argv));
    strcpy(path, *argv);
  }

  dir = opendir(path);
  if( dir == NULL ) {
    fprintf(stderr, "Failed to open directory %s\n", path);
    return -1;
  }

  while( (direntry = readdir(dir)) ) {
    if( !strcmp(direntry->d_name, "..") || !strcmp(direntry->d_name, ".") )
      continue;
    if( direntry->d_type == DT_DIR )
      printf("dir:  %s\n", direntry->d_name);
    else
      printf("file: %s\n", direntry->d_name);
    printHex(direntry->d_name,-1);
    printf("\n");
  }

  closedir(dir);
  free(path);
  return 0;
}

