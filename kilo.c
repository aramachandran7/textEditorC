
/***
PBL based C program.

used to configure terminal to act as a text editor.

Features - error handling, handling and altering terminal settings, displaying text output.

guide:
https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html
@ Window Size, the hard way

PBL C and C++
https://github.com/tuvtran/project-based-learning#cc

 ***/


/*** includes ***/
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>


/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)


/*** data ***/

struct editorConfig {
  int screenrows;
  int screencols;
  struct termios orig_termios;
};
struct editorConfig E;


struct editorConfig E;

/*** terminal (low level) ***/


// error out function
void die(const char *s){

  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}



// this function at a low level reads key presses.
char editorReadKey(){
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

int getWindowSize(int *rows, int *cols){
  struct winsize ws;

  if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    editorReadKey();
    return -1;
  }else{
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}


//resets the terminal settings.
void disableRawMode(){
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}


void enableRawMode(){

  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcsetattr");
  // here we grab the terminal settings file as a struct, change the echo printing value, and
  //  save the terminal settings file.
  atexit(disableRawMode);
  // flipping the echo bit, and setting the local flag.

  struct termios raw = E.orig_termios;
  // turn off echo (displaying inptu) linebyline input, ctrl v, turn off ctrl z/y
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  // turn off ctrl M, ctrl s and q
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);


  raw.c_cflag &= (CS8);
  //turn off output processing - \r\n for any newline.
  raw.c_oflag &= ~(OPOST);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 25;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}


/*** Input (high level) ***/
void editorProcessKeypress(){
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;

  }
}

/*** Output (high level) ***/


void editorDrawRows(){
  int y;
  for (y=0; y<E.screenrows; y++){
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}


void editorRefreshScreen(){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  editorDrawRows();

  write(STDOUT_FILENO, "\x1b[H",3);
}


/*** init ***/

void initEditor(){
  if (getWindowSize(&E.screenrows, &E.screencols)==-1) die("getWindowSize");
}


int main(){
    enableRawMode();
    initEditor();

    while (1){
      // char c = '\0';
      // if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
      // if (iscntrl(c)){
      //   printf("%d\r\n",c);
      // }else{
      //   printf("%d ('%c')\r\n",c,c);
      // }
      // if (c== CTRL_KEY('q')) break;

      editorRefreshScreen();
      editorProcessKeypress();
      // char c = editorReadKey();
      // if (iscntrl(c)){
      //   printf("%d\r\n",c);
      // }else{
      //   printf("%d ('%c')\r\n",c,c);
      // }
  }
  return 0;
}
