#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "cmdline.h"
#include "libgrt_util.h"

vector<vector<const char*>> cmds = {
  {"help",        "h",   "prints this message or the help for the specified command"},
  {"info",        "i",   "print stats about a dataset file"},
  {"train",       "t",   "trains a prediction model"},
  {"predict",     "p",   "predict from unseen data"},
  {"score",       "s",   "calculate classifcation score for prediction"},
  {"extract",     "e",   "extract features from a data sequence"},
  {"preprocess",  "pp",  "preprocess data sequence"},
  {"postprocess", "pop", "postprocess label streams"},
  {"plot",        "pl",  "python based stream plotter"},
  {"segment",     "sg",  "segments a list of samples into multiple timeseries"},
  {"pack",        "pa",  "pack multiple streams into an .mkv file (using ffmpeg)"},
  {"unpack",      "u",   "unpack first audio and subtitle stream from .mkv file (using ffmpeg)"},
};

int usage(int exit_code, string msg="") {
  cout << "usage: grt <cmd> [options]" << endl;
  cout << "available commands: " << endl;

  for (int i=0; i<cmds.size(); i++) {
    string cmd = cmds[i][0],
        _short = cmds[i][1],
          desc = cmds[i][2];

    cout << " " << cmd << "[" << _short << "]" << " - " << desc << endl;
  }

  if (msg != "")
    cerr << endl << "error: " << msg << endl;

  return exit_code;
}


int main(int argc, char *argv[]) 
{
  char *goal = argv[1];

  if (argc <= 1 || (argc <= 2 && strcmp("help",argv[1])==0))
    return usage(0, "missing arguments");

  if (strcmp(argv[1], "help")==0) {
    char executable[256]; snprintf(executable,sizeof(executable),"grt-%s",argv[2]);
    char *const args[] = {"man", executable, (char*) NULL};
    int err = execvp("man", args);
    if (err) cerr << "exec failed: " << strerror(err) << endl;
    return err;
  }

  for (int i=1; i<cmds.size(); i++) {
    const char *cmd=cmds[i][0], *shorthand=cmds[i][1];
    if (strcmp(cmd,argv[1])==0 || strcmp(shorthand,argv[1])==0) {
      char executable[256];
      snprintf(executable,sizeof(executable),"grt-%s",cmd);
      return execvp(executable, &argv[1]);
  }}

  return usage(-1, "command not found");
}
