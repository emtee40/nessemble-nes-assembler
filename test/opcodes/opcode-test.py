#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103, C0301, R0912, R0915, W0612
"""Opcode test"""

import filecmp
import os
import subprocess
import sys
import tempfile

def which(program):
    """Find program path"""

    def is_exe(fpath):
        """Detect executable"""

        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)

    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)

            if is_exe(exe_file):
                return exe_file

    return None

def main():
    """Main function"""

    root = os.getcwd()
    dirname = os.path.dirname(os.path.realpath(__file__))

    try:
        opcode = sys.argv[1]
    except IndexError:
        exit(1)

    try:
        flags = sys.argv[2:]
    except IndexError:
        flags = None

    valgrind = which('valgrind')

    if not opcode:
        exit(1)

    # assemble
    if os.path.exists('%s%s%s%s%s.rom' % (dirname, os.sep, opcode, os.sep, opcode)):
        arguments = ['%s%snessemble' % (root, os.sep), '%s%s%s%s%s.asm' % (dirname, os.sep, opcode, os.sep, opcode), '--output', '-']
        if flags:
            arguments.extend(flags)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stds = child.communicate()
        rc = child.returncode

        data = ''

        if stds[1]:
            data = stds[1]

        data += stds[0]

        tmp = tempfile.NamedTemporaryFile()
        tmp.write(data)
        tmp.seek(0)

        if not filecmp.cmp('%s%s%s%s%s.rom' % (dirname, os.sep, opcode, os.sep, opcode), tmp.name):
            tmp.close()
            print 'failed assembly'
            print ' '.join(arguments)
            exit(1)

        tmp.close()

        # assemble (valgrind)
        if valgrind:
            arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s%s%s%ssuppressions.supp' % (root, os.sep, 'test', os.sep), '--error-exitcode=2', '-q', '%s%snessemble' % (root, os.sep), '%s%s%s%s%s.asm' % (dirname, os.sep, opcode, os.sep, opcode), '--output', '-']
            if flags:
                arguments.extend(flags)

            child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data = child.communicate()[0]
            rc = child.returncode

            if rc == 2:
                print 'memory leak assembly'
                print ' '.join(arguments)
                exit(rc)

    # disassemble
    if os.path.exists('%s%s%s%s%s-disassembled.txt' % (dirname, os.sep, opcode, os.sep, opcode)):
        arguments = ['%s%snessemble' % (root, os.sep), '%s%s%s%s%s.rom' % (dirname, os.sep, opcode, os.sep, opcode), '--disassemble', '--output', '-']
        if flags:
            arguments.extend(flags)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stds = child.communicate()
        rc = child.returncode

        data = ''

        if stds[1]:
            data = stds[1]

        data += stds[0]

        tmp = tempfile.NamedTemporaryFile()
        tmp.write(data)
        tmp.seek(0)

        if not filecmp.cmp('%s%s%s%s%s-disassembled.txt' % (dirname, os.sep, opcode, os.sep, opcode), tmp.name):
            tmp.close()
            print 'failed disassembly'
            print ' '.join(arguments)
            exit(1)

        tmp.close()

        # disassemble (valgrind)
        if valgrind:
            arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s%s%s%ssuppressions.supp' % (root, os.sep, 'test', os.sep), '--error-exitcode=2', '-q', '%s%snessemble' % (root, os.sep), '%s%s%s%s%s.rom' % (dirname, os.sep, opcode, os.sep, opcode), '--disassemble', '--output', '-']
            if flags:
                arguments.extend(flags)

            child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data = child.communicate()[0]
            rc = child.returncode

            if rc == 2:
                print 'memory leak disassembly'
                print ' '.join(arguments)
                exit(rc)

    # simulate
    if os.path.exists('%s%s%s%s%s-simulated.txt' % (dirname, os.sep, opcode, os.sep, opcode)):
        arguments = ['%s%snessemble' % (root, os.sep), '--simulate', '%s%s%s%s%s.rom' % (dirname, os.sep, opcode, os.sep, opcode), '--recipe', '%s%s%s%s%s-recipe.txt' % (dirname, os.sep, opcode, os.sep, opcode), '--output', '-']
        if flags:
            arguments.extend(flags)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stds = child.communicate()
        rc = child.returncode

        data = ''

        if stds[1]:
            data = stds[1]

        data += stds[0]

        tmp = tempfile.NamedTemporaryFile()
        tmp.write(data)
        tmp.seek(0)

        if not filecmp.cmp('%s%s%s%s%s-simulated.txt' % (dirname, os.sep, opcode, os.sep, opcode), tmp.name):
            tmp.close()
            print 'failed simulation'
            print ' '.join(arguments)
            exit(1)

        tmp.close()

        # simulate (valgrind)
        if valgrind:
            arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s%s%s%ssuppressions.supp' % (root, os.sep, 'test', os.sep), '--error-exitcode=2', '-q', '%s%snessemble' % (root, os.sep), '--simulate', '%s%s%s%s%s.rom' % (dirname, os.sep, opcode, os.sep, opcode), '--recipe', '%s%s%s%s%s-recipe.txt' % (dirname, os.sep, opcode, os.sep, opcode), '--output', '-']
            if flags:
                arguments.extend(flags)

            child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data = child.communicate()[0]
            rc = child.returncode

            if rc == 2:
                print 'memory leak simulation'
                print ' '.join(arguments)
                exit(rc)

if __name__ == '__main__':
    main()
