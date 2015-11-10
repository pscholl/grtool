#!/usr/bin/env python3
import sys, os, re, difflib, shutil
from subprocess import Popen, PIPE, call
from tempfile import mkdtemp
from argparse import ArgumentParser

def run_test(test, PATH, gdb=False):
    """ run some bash test. Syntax is very simplitic, everything that is intended by
    4 spaces is a test. Pull these groups out, the first line + every line after that
    that is prefixed with ">" are command and arguments. Everything after that is the
    expected output.
    """
    print ("%s:" % test)

    cmd_results = []
    with open(test) as f:
        re_codeblock = re.compile("\n\s*\n {4}(.*\n(?: {4}>.*\n)*)(( {4}.*\n)*)($|\s{0,3})")
        cmd_results  = [ (m.group(1).lstrip(),m.group(2).lstrip()) for m in re_codeblock.finditer(f.read()) ]

    if PATH is not None:
        if PATH[0] != "/": PATH = os.path.join(os.getcwd(), PATH)
        os.environ["PATH"]="%s:%s"%(PATH,os.environ["PATH"])

    cwd, num, fail = os.getcwd(), 0, False
    for cmd, res in cmd_results:
        # create a temp dir to work in
        tdir=mkdtemp()
        os.rmdir(tdir)
        shutil.copytree(os.path.join(cwd,os.path.dirname(test)),tdir)
        os.chdir(tdir)

        cmd = cmd.replace("\n    > ", "\n").replace("\n    >","\n").strip()
        res = res.replace("\n    ", "\n")
        p   = Popen(cmd,shell=True,executable='/bin/bash',stdout=PIPE,stderr=PIPE,universal_newlines=True)
        err = p.wait()
        out = p.stdout.read()

        if err != 0:
            fail = True
            print ("  Test #%d: FAILED" % num)
            print ("  cmd '%s' failed with code: %d"% (cmd, err))
            serr = p.stderr.read()
            sys.stderr.write(serr)
            if gdb and "core dumped" in serr: call("coredumpctl gdb -1", shell=True)
        elif out != res and res.strip()!="":
            fail = True
            print ("  cmd '%s' FAILED:"% cmd)
            d = difflib.Differ().compare(res.split("\n"),out.split("\n"))
            print("\n".join(list(d)))
            serr = p.stderr.read()
            sys.stderr.write(serr)
        else:
            print ("  Test #%d: OK" % num)

        shutil.rmtree(tdir)
        num += 1
    os.chdir(cwd)
    return fail

if __name__ == "__main__":
    p = ArgumentParser("run bash code in markdown documents")
    p.add_argument('files', nargs='+', type=str)
    p.add_argument('-p', '--path', type=str)
    p.add_argument('-g', '--gdb', action="store_true", help="run gdb on core dumping tests")
    args = p.parse_args()

    sys.exit(any(run_test(test, args.path, args.gdb) for test in args.files))
