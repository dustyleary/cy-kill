#!/usr/bin/env python

import json
import os
import re
import subprocess
import sys

import sgf

def good_rank(rank):
    if not rank:
        return False
    if rank.endswith('p'):
        return True
    if not rank.endswith('d'):
        return False
    return int(rank[:-1]) >= 5

def want_game(semis):
    #19x19 board
    if semis[0]['SZ'][0] != '19':
        return False
    #high rank
    b = good_rank(semis[0].get('BR', [''])[0])
    w = good_rank(semis[0].get('WR', [''])[0])
    if not (b or w):
        return False
    #no handicap
    if 'AB' in semis[0]:
        return False
    if 'AW' in semis[0]:
        return False
    return True, b, w

def out(t):
    print>>sys.stderr,t
    sys.stderr.flush()

def main():
    dir = sys.argv[1]
    def visit(sgf_files, dirname, names):
        sf = [n for n in names if n.endswith('.sgf')]
        sgf_files.extend([os.path.join(dirname, n) for n in sf])
    sgf_files = []
    os.path.walk(dir, visit, sgf_files)
    out("sgf_files: %d" % len(sgf_files))

    parsed = []
    for i,filename in enumerate(sgf_files):
        if i and not i%1000:
            out("sgf: %d/%d" % (i, len(sgf_files)))
        text = file(filename).read()
        semis = sgf.parse(text)
        wg = want_game(semis)
        if wg:
            parsed.append([semis, wg])

    out("good files: %d" % len(parsed))
    json.dump(parsed, sys.stdout)

if __name__ == '__main__':
    main()


