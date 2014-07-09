#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <byteswap.h>

#define TUPELSOFEIGHT 2

void print_hex(char* s, int len) {
  if( len == 0 )
    return;
  int i = 0;
  if( len < 0 )
    i = strlen(s);
  while( i < len ) {
    printf("%02x ", (unsigned char)s[i]);
    i++;
  }
}

int main(int argc, char** argv) {
  if( argc != 3 && argc != 4 ) {
    printf("usage: %s <file1> <file2> [n]\n", *argv);
    exit(1);
  }

  argv++;
  FILE* file1 = fopen(*argv,"r");
  if( !file1 ) {
    printf("failed to open file1 %s\n", *argv);
    exit(1);
  }

  argv++;
  FILE* file2 = fopen(*argv,"r");
  if( !file2 ) {
    printf("failed to open file1 %s\n", *argv);
    exit(1);
  }

  bool limit = false;
  long int lines = 0;
  if( argc == 4 ) {
    argv++;
    sscanf(*argv, "%ld", &lines);
    printf("printing only %ld lines\n", lines);
    limit = true;
  }

  while( feof(file1)+feof(file2) == 0 ) {
    uint64_t buf1[TUPELSOFEIGHT];
    int bytes_read1 = fread(buf1, 1, TUPELSOFEIGHT*sizeof(uint64_t), file1);
    if( bytes_read1 == 0 ) {
      printf("failed to read from file1 at %#018lx (%ld) trying to read %ld bytes\n", ftell(file1), ftell(file1), TUPELSOFEIGHT*sizeof(uint64_t));
      exit(1);
    }
    uint64_t buf2[TUPELSOFEIGHT];
    int bytes_read2 = fread(buf2, 1, TUPELSOFEIGHT*sizeof(uint64_t), file2);
    if( bytes_read2 == 0 ) {
      printf("failed to read from file2 at %#018lx (%ld) trying to read %ld bytes\n", ftell(file2), ftell(file2), TUPELSOFEIGHT*sizeof(uint64_t));
      exit(1);
    }
    uint64_t xor[TUPELSOFEIGHT];
    for( int i = 0; i < TUPELSOFEIGHT; i++ )
      xor[i] = buf1[i] ^ buf2[i];
      //xor[i] = bswap_64(buf1[i] ^ buf2[i]);
    printf("%#018lx: ", ftell(file1)-8*TUPELSOFEIGHT);
    print_hex((char*)xor,(bytes_read2>bytes_read1)?bytes_read1:bytes_read2);
    printf("\n");
    if( limit ) {
      lines -= 1;
      if( lines <= 0 )
        break;
    }
    if( bytes_read1 != bytes_read2 ) //stop because one file is at its end
      break;
  }
}
