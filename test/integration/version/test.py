#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103,C0301
"""version test"""

import os
import subprocess

def main():
    """Main function"""

    dirname = os.path.dirname(os.path.realpath(__file__))
    example = 'version'

    arguments = ['python', '%s%s..%sintegration-test.py' % (dirname, os.sep, os.sep), example, '--version']
    child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    data = child.communicate()[0]
    rc = child.returncode

    if rc != 0:
        print data

    exit(rc)

if __name__ == '__main__':
    main()