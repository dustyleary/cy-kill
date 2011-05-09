#!/usr/bin/env python

import json
import os
import re
import subprocess
import sys

def sgf_vert_to_gtp_vert(v):
    if not v:
        return "pass"
    y = ord(v[1]) - ord('a')
    y = 19 - y
    x = v[0]
    if ord(v[0]) > ord('h'):
        x = chr(ord(v[0])+1)
    return x+str(y)

def get_pattern_input(size, semis, b, w):
    lines = []
    lines.append("boardsize 19")
    def do_move(c, v):
        lines.append("echo_text PATTERN SIZE %d MOVE START" % (size,))
        lines.append("pattern_at %s %s %d" % (c, sgf_vert_to_gtp_vert(v), size))
        lines.append("echo_text PATTERN ALLMOVES")
        for y in range(19):
            for x in range(19):
                ov = chr(x+ord('a'))+chr(y+ord('a'))
                lines.append("pattern_at %s %s %d" % (c, sgf_vert_to_gtp_vert(ov), size))
        lines.append("echo_text PATTERN MOVE END")
    for m in semis[1:]:
        c = m.keys()[0]
        v = m[c][0]
        if ((c == 'W') and w) or ((c=='B') and b):
            do_move(c,v)
        lines.append("play %s %s" % (c, sgf_vert_to_gtp_vert(v)))

    text = "\n".join(lines)
    return text

def out(t):
    print>>sys.stderr,t
    sys.stderr.flush()

def main():
    filename = sys.argv[1]
    out("loading games...")
    parsed = json.load(file(filename, 'rt'))

    out("loaded: %d" % len(parsed))
    return

    p = subprocess.Popen("./cy-kill", stdin=subprocess.PIPE, stdout=sys.stdout)

    print >>p.stdin, "buffer_io 1"
    for i,(semis, (_,b,w)) in enumerate(parsed):
        out("games: %d/%d" % (i, len(parsed)))
        pinput = get_pattern_input(size, semis, b, w)
        print >>p.stdin, pinput
    print >>p.stdin, "quit"
    p.wait()

if __name__ == '__main__':
    main()

