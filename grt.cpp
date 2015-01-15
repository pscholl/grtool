#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "cmdline.h"
#include "libgrt_util.h"

int main(int argc, char *argv[]) 
{
  cmdline::parser c;
  c.add("help", 'h', "print this message");
  c.footer("cmd [args]...");

  /* which args are for us? */
  int newargc = 1;

  while (newargc < argc && argv[newargc][0] == '-')
    newargc++;

  /* parse the classifier-common arguments */
  if (!c.parse(newargc,argv,false)) {
    cerr << c.usage() << "\n" << c.error() << "\n" ;
    return -1;
  }

  if (c.exist("help")) {
    cerr << c.usage() << endl;
    return -1;
  }

  if (argc == newargc) {
    cerr << "error: please provide command" << endl;
    cerr << c.usage() << endl;
  }

  char cmd[256];
  snprintf(cmd,sizeof(cmd),"./grt-%s",argv[newargc]);
  int err = execvp(cmd, &argv[newargc++]);

  cout << "command execution failed: " << strerror(err) << endl;

  return -1;
}
