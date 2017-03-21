#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "cmdline.h"
#include "libgrt_util.h"

vector<vector<const char*>> cmds = {
  {"help",        "h",   "prints this message or the help for the specified command"},
  {"info",        "i",   "print stats about a dataset file"},
  {"train",       "t",   "trains a prediction model"},
  {"train-dlib",  "td",  "trains a prediction model, uses dlib multiclass machine learning trainers"},
  {"train-skl",   "ts",  "trains a prediction model, uses scikit-learn unsupervised estimators"},
  {"predict",     "p",   "predict from unseen data"},
  {"predict-dlib","pd",  "predict from unseen data, processes trainers created from train-dlib"},
  {"predict-skl", "ps",  "predict from unseen data, processes models created from train-skl"},
  {"score",       "s",   "calculate classifcation score for prediction"},
  {"score-tool",  "st",  "scoring tool for analyzing multiple [grt score] results"},
  {"extract",     "e",   "extract features from a data sequence"},
  {"preprocess",  "pp",  "preprocess data sequence"},
  {"postprocess", "pop", "postprocess label streams"},
  {"plot",        "pl",  "python based stream plotter"},
  {"montage",     "m",   "python based montage plot"},
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

const char* expand(char *arg) {
  for (size_t i=0; i<cmds.size(); i++)
    if (strcmp(cmds[i][1], arg)==0)
      return cmds[i][0];
  return arg;
}


int main(int argc, char *argv[])
{
  char *goal = argv[1];

  if (argc <= 1 || (argc <= 2 && strcmp("help",expand(argv[1]))==0))
    return usage(0, "missing arguments");

  if (strcmp(expand(argv[1]), "help")==0)
  {
    char executable[256];
    char *const args[] = {"man", executable, (char*) NULL};
    snprintf(executable,sizeof(executable),"grt-%s",expand(argv[2]));
    int err = execvp("man", args);
    if (err) cerr << "exec failed: " << strerror(err) << endl;
    return err;
  }
  else
  {
    char executable[256];
    snprintf(executable,sizeof(executable),"grt-%s",expand(argv[1]));
    int err = execvp(executable, &argv[1]);
    if (err) {
      snprintf(executable,sizeof(executable),"unable to exec 'grt-%s': ",expand(argv[1]));
      perror(executable);
    }
    return err;
  }
}
