#!/usr/bin/env python2
#
# styler.py - check or fix code style in files recursively
# Usage: styler.py --fix|--check directory
#
# Dependencies:
#  - astyle http://astyle.sourceforge.net/ version 2 or later
#  - diff https://www.gnu.org/software/diffutils/
#
# Copyright 2015 Grigory Rechistov <grigory.rechistov@phystech.edu>
#
# Released under the BSD license: http://opensource.org/licenses/BSD-2-Clause
#

import os
import sys
import subprocess
import argparse

verbose = False # XXX this global is as lame as me

def name_is_blacklisted(fname):
    blacklist = ["modules/sdl"]
    for entry in blacklist:
        if fname.find(entry) != -1:
            return True
    return False

def operate_on_file(fname, do_fix = False):
    extension = fname.split('.')[-1]
    if extension != 'c' and extension != 'h':
        if verbose: print "Ignored non-C file %s" % fname
        return 0
    if name_is_blacklisted(fname):
        if verbose: print "Blacklisted file %s" % fname
        return 0
    # Check style
    process = subprocess.Popen(["./tools/check_c_style.sh",
                               fname],
                               stderr=subprocess.PIPE,
                               stdout=subprocess.PIPE)
    (out, errout) = process.communicate()
    exitcode = process.wait()
    if exitcode != 1 and exitcode != 0:
        print ("Internal error:",
              "unexpected exit code %d from check_c_style.sh for %s" % \
              (exitcode, fname))
        exit(127)
    if exitcode == 1:
        if do_fix:
            print "Reformatting %s" % fname
            print "FIXME: reformatting is not implemented"
            return 0
        else:
            print "Style problems in %s" % fname
        return 1
    return 0

def main():
    do_fix = False
    glob_check = False # True

    parser = argparse.ArgumentParser(description=\
            'Check or fix code style in files recursively.')
    parser.add_argument('--fix', action='store_true', default = False,
                   help='Fix problems by changing files')
    parser.add_argument('--all', action='store_true', default = False,
                   help='Operate on all files, not only modified')
    parser.add_argument('rootDir', type=str,
                   help='Top level directory name')
    parser.add_argument('--verbose', action='store_true', default=False,
                   help='Be verbose')
    parser.add_argument('--noerror', action='store_true', default = False,
                   help='Exit with code 0 even if there are problems')

    args = parser.parse_args()

    do_fix = args.fix
    glob_check = args.all
    rootDir = args.rootDir
    verbose = args.verbose

    bad_style = 0
    if glob_check:
        # Traverse through the tree and check all files
        for dirName, subdirList, fileList in os.walk(rootDir):
            for fn in fileList:
                fname = os.path.join(dirName, fn)
                bad_style += operate_on_file(fname, do_fix)
    else:
        # Get a list of files changed against remote master:
        # git diff --name-status origin/master
        # M       modules/chip16/chip16.gi
        # D       test/chip16/unit-tests/subi/SUITEINFO
        # Note: the algorithm below is not foolproof:
        # files with spaces in names might cause an error
        gitdiff = subprocess.Popen(["git", "diff",
                               "--name-status",
                               "origin/master",
                               "--",
                               rootDir],
                               stderr=subprocess.PIPE,
                               stdout=subprocess.PIPE)
        (out, errout) = gitdiff.communicate()
        exitcode = gitdiff.wait()
        if exitcode != 0:
            print "Git error: %s" % errout
            exit(127)
        out = out.split('\n')
        for line in out:
            # print line
            if len(line) == 0: continue
            cmd = line[0]
            fname = line[1:].strip()
            # Check added, modified, renamed files
            if cmd == "A" or cmd == "M" or cmd == "R":
                bad_style += operate_on_file(fname, do_fix)

    if bad_style > 0:
        print
        print "Style problems with %d files" % bad_style
        if args.noerror: return 0
        return 1
    else:
        if verbose: print "No style problems"
        return 0

if __name__ == "__main__":
    exit(main())
