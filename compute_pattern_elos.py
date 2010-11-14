import sys
import os
import re
import sqlite3

def compute_expected(a, b):
    a = float(a)
    b = float(b)
    qa = 10**(a/400)
    qb = 10**(b/400)
    ea = qa / (qa+qb)
    eb = qb / (qa + qb)
    return (ea, eb)

def compute_update(a, b, k): #a wins
    a = float(a)
    b = float(b)
    k = float(k)
    ea, eb = compute_expected(a, b)
    sa, sb = 1.0, 0.0
    na = a + k*(sa-ea)
    nb = b + k*(sb-eb)
    return (na, nb)

def out(t):
    print>>sys.stderr,t
    sys.stderr.flush()

def get_competitions(lines):
    while True:
        line = lines.next()
        if re.match("= PATTERN SIZE \d+ MOVE START", line):
            winner = lines.next()
            if not winner.startswith("= :"): #pass
                while True:
                    line = lines.next()
                    if re.match("= PATTERN MOVE END", line):
                        break
                continue;
            winner = winner[3:]
            losers = set()
            while True:
                line = lines.next()
                if re.match("= PATTERN MOVE END", line):
                    break
                if line.startswith("= :"):
                    loser = line[3:]
                    losers.add(loser)
            losers.remove(winner)
            yield (winner, losers)

def goodlines(lines):
    while True:
        line = lines.next()
        line = line.strip()
        if line:
            yield line

def main():
    outfile = sys.argv[1]

    conn = sqlite3.connect(outfile)
    c = conn.cursor()
    try:
        c.execute(r"create table patterns (pattern text primary key, gamma real)")
    except Exception:
        pass
    all_moves = {}

    competition_gen = get_competitions(goodlines(sys.stdin.xreadlines()))
    for i,comp in enumerate(competition_gen):
        if 999 == i%1000:
            out('moves: %d' % (i+1))
            conn.commit()
        winner, losers = comp
        for loser in losers:
            a, b = all_moves.setdefault(winner, 1500.0), all_moves.setdefault(loser, 1500.0)
            na, nb = compute_update(a, b, 10)
            all_moves[winner], all_moves[loser] = na, nb
            c.execute(r"replace into patterns values ('%s', %f)" % (winner, na))
            c.execute(r"replace into patterns values ('%s', %f)" % (loser, nb))

    c.execute("select * from patterns order by gamma desc")
    for row in c:
        print row

if __name__ == '__main__':
    main()

