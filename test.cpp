#include "libgrt_util.h"

using namespace std;

int main(int argc, char *argv[]) {
  CsvIOSample io("classification");

  while( cin >> io )
    cerr << "got " << endl;
}
