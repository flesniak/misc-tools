//find files with broken utf8 in their name and print, rename or delete them

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

static bool verbose = false;
static bool renameFiles = false;
static bool deleteFiles = false;

void printHex(char* string, int len) {
  if( len == 0 )
    return;
  int i = 0;
  while( string[i] != 0 ) {
    printf("%x ", (unsigned char)string[i]);
    i++;
    if( i == len )
      break;
  }
}

//checks if name is valid utf8
//returns length of string if no errors were found, otherwise index of invalid character
int checkName(char* n) {
  const int l = strlen(n);
  int i = 0;
  int ubytes = 0;
  unsigned char* name = (unsigned char*)n;
  while( i < l ) {
    //ascii characters
    if( name[i] < 0x7F ) {
      //if we still need unicodes-bytes but we found a ascii character, invalid!
      if( ubytes > 0 )
        break;
    }
    //we search for extended unicode bytes, so every char >= 0x80 is valid (some exceptions apply, but ignored here for simplicity)
    if( ubytes > 0 && name[i] >= 0x80 )
      ubytes--;
    else {
      //values between 0x7F and 0xC2 are reserved, invalid!
      if( name[i] > 0x7F && name[i] < 0xC2 )
        break;
      //values > 127 initiate utf8
      if( name[i] >= 0xC2 ) {
        ubytes++;
        if( name[i] >= 0xE0 ) {
          ubytes++;
          if( name[i] >= 0xF0 )
            ubytes++;
        }
      }
    }
    i++;
  }
  return i;
}

void toValidName(char* n) {
  int pos = checkName(n);
  while( pos < strlen(n) ) {
    memmove(n+pos,n+pos+1,strlen(n)-pos);
    pos = checkName(n);
  }
}

int processDirectory(char* path) {
  DIR* dir;
  struct dirent* direntry;

  dir = opendir(path);
  if( dir == NULL ) {
    fprintf(stderr, "failed to open directory %s\n", path);
    return 0;
  }

  int found = 0;

  while( (direntry = readdir(dir)) ) {
    if( !strcmp(direntry->d_name, "..") || !strcmp(direntry->d_name, ".") )
      continue;
    if( direntry->d_type == DT_DIR ) {
      char* sub = malloc(strlen(path)+2+strlen(direntry->d_name));
      sub[0] = 0;
      strcat(sub, path);
      strcat(sub, "/");
      strcat(sub, direntry->d_name);
      int c = checkName(direntry->d_name);
      if( c == strlen(direntry->d_name) ) {
        found += processDirectory(sub);
      } else {
        found++;
        if( renameFiles ) {
          printf("subdirectory contains errors, renaming: %s\n", sub);
          if( verbose ) {
            printHex(sub,-1);
            printf("\n");
          }

          char* newname = malloc(strlen(direntry->d_name));
          strcpy(newname,direntry->d_name);
          toValidName(newname);
          if( verbose )
            printf("renaming this dir to %s\n", newname);

          char* newsub = malloc(strlen(path)+2+strlen(newname));
          newsub[0] = 0;
          strcat(newsub, path);
          strcat(newsub, "/");
          strcat(newsub, newname);
          free(newname);
          if( rename(sub, newsub) )
            printf("failed to rename directory %s\n", sub);
          else {
            found += processDirectory(newsub);
          }
          free(newsub);
        } else {
          if( deleteFiles ) //don't delete invalid directories, data inside may still be of use ;)
            printf("subdirectory contains errors, skipping but won't delete: %s\n", sub);
          else
            printf("subdirectory contains errors, skipping: %s\n", sub);
          if( verbose ) {
            printHex(sub,-1);
            printf("\n");
          }
        }
      }
      free(sub);
    } else {
      int i = checkName(direntry->d_name);
      if( i != strlen(direntry->d_name) ) {
        printf("invalid: %s/%s", path, direntry->d_name);
        if( verbose ) {
          printf(": has invalid unicode at %d, hex below:\n", i);
          printHex(direntry->d_name,-1);
        }
        printf("\n");

        char* sub = 0;
        if( renameFiles || deleteFiles ) { //build path for both delete and rename functions
          sub = malloc(strlen(path)+2+strlen(direntry->d_name));
          sub[0] = 0;
          strcat(sub, path);
          strcat(sub, "/");
          strcat(sub, direntry->d_name);
        }

        if( renameFiles ) {
          char* newname = malloc(strlen(direntry->d_name)+1);
          strcpy(newname,direntry->d_name);
          toValidName(newname);
          if( verbose )
            printf("renaming this file to %s\n", newname);
          char* newsub = malloc(strlen(path)+2+strlen(newname));
          newsub[0] = 0;
          strcat(newsub, path);
          strcat(newsub, "/");
          strcat(newsub, newname);
          free(newname);
          if( rename(sub, newsub) )
            printf("failed to rename file %s\n", sub);
          free(newsub);
        }

        if( deleteFiles ) {
          if( remove(sub) )
            printf("removed %s\n", sub);
          else
            printf("failed to remove %s\n", sub);
        }

        if( renameFiles || deleteFiles )
          free(sub);

        found++;
      }
    }
  }

  closedir(dir);
  return found;
}

void usage() {
  printf("usage: fixbrokenfilenames [-d|-r] [-v] [directory]\n");
  exit(1);
}

int main(int argc, char** argv) {
  if( argc > 4 )
    usage();
  argc--;
  argv++;

  char* path = 0;
  while( argc ) {
    if( !strcmp(*argv, "-d") ) {
      deleteFiles = true;
      printf("will delete invalid files!\n");
    } else {
      if( !strcmp(*argv, "-v") ) {
        verbose = true;
        printf("will be verbose\n");
      } else {
        if( !strcmp(*argv, "-r") ) {
          renameFiles = true;
          printf("will rename invalid files!\n");
        } else {
          if( path != 0 ) {
            free(path);
            usage();
          } else {
            path = malloc(strlen(*argv));
            strcpy(path, *argv);
          }
        }
      }
    }
    argc--;
    argv++;
  }

  if( deleteFiles && renameFiles ) {
    printf("deleting and renaming is not possible at once\n");
    usage();
  }

  if( path == 0 ) {
    path = malloc(2);
    strcpy(path, ".");
  }

  int found = processDirectory(path);

  printf("found %d invalid files and directories\n", found);

  free(path);
  return 0;
}

