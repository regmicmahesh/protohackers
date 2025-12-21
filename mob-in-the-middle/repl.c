#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TONY_ADDRESS "7YWHMfk9JZe0LM0g1ZauHuiSxhI"

int check_valid_address(char *xs) {
  int length = strlen(xs);

  if (length < 26 || length > 35) {
    return 0;
  };

  if (*xs != '7') {
    return 0;
  }

  for (int i = 0; i < length; i++) {
    if (!isalnum(xs[i])) {
      return 0;
    }
  }
  return 1;
}

int checkWord(char *xs) {
  int length = strlen(xs);

  if (length < 26 || length > 35) {
    return 0;
  };

  if( *xs != '7'){
    return 0;
  }

  for(int i = 0; i < length; i++){
    if(!isalnum(xs[i])) {
        return 0;
    }
  }
  return 1;
}


char *fix_message(char *xs) {
  int newline_flag = 0;
  char *newBuf = malloc(strlen(xs) * 2);
  memset(newBuf, 0, strlen(xs) * 2);

  if (xs[strlen(xs) - 1] == '\n') {
    newline_flag = 1;
    xs[strlen(xs) - 1] = '\0';
  }

  char *ptr = strtok(xs, " ");

  while (ptr != NULL) {
    if (check_valid_address(ptr)) {
      strcat(newBuf, TONY_ADDRESS);
    } else {
      strcat(newBuf, ptr);
    }
    strcat(newBuf, " ");
    ptr = strtok(NULL, " ");
  };

  // consume the extra space in the end.
  newBuf[strlen(newBuf)-1] = '\0';

  if (newline_flag) {
    strcat(newBuf, "\n");
  }

  printf("%s\n", newBuf);

  return newBuf;

}

int main() {
  char msgOriginal[1024] = "a";
  char *message = &msgOriginal;
  char *x = strsep(&message, " ");
  while(x) {
  printf("%s\n", x);
  x = strsep(&message, " ");
  }
  // char *res = fix_message(message);
  // printf("|%s|\n", res);
}