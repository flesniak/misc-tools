#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
  if( argc != 2 ) {
    printf("Usage: hextofile <hex-encoded data>\n");
    exit(1);
  }

  argv++;
  uint offset=0;
  uint size=strlen(*argv);
  size -= size%2; //we can only handle 2n bytes
  while( offset < size ) {
    char snippet[2];
    char byte;
    strncpy(snippet,*argv+offset,2);
    sscanf(snippet,"%X",&byte);
    putchar(byte);
    offset+=2;
  }
}
