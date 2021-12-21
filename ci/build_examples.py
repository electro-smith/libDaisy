#!/usr/bin/env python
#
# recompiles all Make-based projects within the repository
# excluding any that are located within the specified excluded
# directory list
#
import sys
import os
import argparse

parser = argparse.ArgumentParser(description='Builds several examples using Makefile build system')
parser.add_argument('-d', '--start_dir', help='specifies the path to the directory to start searching', default='.')
parser.add_argument('-f', '--extra_filters', nargs='*', help='space separated list of additional folders to ignore.')
args = parser.parse_args()

if not os.path.isdir(args.start_dir):
    print("chosen path is not a directory")
    quit()
dir_to_start = os.path.abspath(args.start_dir)

filter_dirs = ["libDaisy",
               "DaisySP",
               ".github",
               ".vscode",
               ".git",
               "ci",
               "cube",
               "dist",
               "utils",
               "stmlib",
               "libdaisy",
               "MyProjects"]

if args.extra_filters:
    filter_dirs.extend(args.extra_filters)

dirs_to_search = list(
    filter(lambda x: x not in filter_dirs and os.path.isdir(x), os.listdir(dir_to_start)))

# recursively go through each directory in dirs_to_search
# and attempt to compile each example
for dir in dirs_to_search:
    example_dirs = []
    for root, dirs, files in os.walk(dir):
        if 'Makefile' in files:
            example_dirs.append(root)
    cwd = os.path.abspath(os.getcwd())
    for ex in example_dirs:
        dest = os.path.join(cwd, ex)
        os.chdir(dest)
        os.system("echo Building: {}".format(ex))
        exit_code = os.system('make -s clean')
        exit_code = os.system('make -s')
        if exit_code != 0:
            os.chdir(cwd)
            sys.exit(1)
    os.chdir(cwd)
# exit successfully
print("done")
sys.exit(0)
