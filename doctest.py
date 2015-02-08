#!/usr/bin/env python
import sys, os, re, difflib
from subprocess import Popen, PIPE
from tempfile import mkdtemp

def run_test(test):
    """ run some bash test. Syntax is very simplitic, everything that is intended by
    4 spaces is a test. Pull these groups out, the first line + every line after that
    that is prefixed with ">" are command and arguments. Everything after that is the
    expected output.
    """
    cmd_results = []
    with open(test) as f:
        re_codeblock = re.compile("\n\s*\n {4}(.*\n(?: {4}\s*>.*\n)+)(( {4}.*\n)+)\s{0,3}\n")
        cmd_results  = [ (m.group(1).lstrip(),m.group(2).lstrip()) for m in re_codeblock.finditer(f.read()) ]


    cwd, num = os.getcwd(), 0
    for cmd, res in cmd_results:
        # create a temp dir to work in
        tdir=mkdtemp()
        os.chdir(tdir)

        cmd = cmd.replace("\n    > ", "\n").replace("\n    >","\n")
        res = res.replace("\n    ", "\n")
        p   = Popen(cmd,shell=True,stdout=PIPE,stderr=PIPE,universal_newlines=True)
        err = p.wait()
        out = p.stdout.read()

        if err != 0:
            print ("Test #%d: FAILED" % num)
            print ("cmd '%s' failed with code: %d"% (cmd, err))
        elif out != res:
            print ("Test #%d: FAILED" % num)
            d = difflib.Differ().compare(res.split("\n"),out.split("\n"))
            print("\n".join(list(d)))
        else:
            print ("Test #%d: OK" % num)

        os.rmdir(tdir)
        num += 1
    os.chdir(cwd)

if __name__ == "__main__":
    for test in sys.argv[1:]:
        print ("Running test %s:" % test)
        run_test(test)
