import sys
import os
import re
import subprocess

def parse_sgf_file(filename):
    text = file(filename).read()
    text = text.strip()
    if not text.startswith("(;GM[1]"):
        raise Exception, "not GM1"
    t = text[1:]
    semis = []
    while True:
        t = t.strip()
        if t[0] == ';':
            semis.append({})
            t = t[1:]
            continue
        if t[0] == ')':
            break
        m = re.match("([A-Z]+)((\s*\[[^\]]*\])+)", t, re.MULTILINE)
        k = m.group(1)
        vs = m.group(2)
        vms = re.findall("\s*\[([^\]]*)\]", vs, re.MULTILINE)
        if k in semis[-1]:
            raise Exception, "duplicate key '%s'" % k
        semis[-1][k] = vms
        t = t[len(m.group(0)):]
    return semis

def good_rank(rank):
    if not rank:
        return False
    if rank.endswith('p'):
        return True
    if not rank.endswith('d'):
        return False
    return int(rank[:-1]) >= 7

def want_game(semis):
    if semis[0]['SZ'][0] != '19':
        return False
    b = good_rank(semis[0].get('BR', [''])[0])
    w = good_rank(semis[0].get('WR', [''])[0])
    if not (b or w):
        return False
    if 'AB' in semis[0]:
        return False
    if 'AW' in semis[0]:
        return False
    return True, b, w

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
    dir = sys.argv[1]
    size = int(sys.argv[2])
    def visit(sgf_files, dirname, names):
        sf = [n for n in names if n.endswith('.sgf')]
        sgf_files.extend([os.path.join(dirname, n) for n in sf])
    sgf_files = []
    os.path.walk(dir, visit, sgf_files)
    out("sgf_files: %d" % len(sgf_files))

    parsed = []
    for i,file in enumerate(sgf_files):
        if i and not i%1000:
            out("sgf: %d/%d" % (i, len(sgf_files)))
        semis = parse_sgf_file(file)
        wg = want_game(semis)
        if wg:
            parsed.append([semis, wg])

    out("good files: %d" % len(parsed))

    p = subprocess.Popen("cy-kill", stdin=subprocess.PIPE, stdout=sys.stdout)

    print >>p.stdin, "buffer_io 1"
    for i,(semis, (_,b,w)) in enumerate(parsed):
        out("games: %d/%d" % (i, len(parsed)))
        pinput = get_pattern_input(size, semis, b, w)
        print >>p.stdin, pinput
    print >>p.stdin, "quit"
    p.wait()

if __name__ == '__main__':
    main()

