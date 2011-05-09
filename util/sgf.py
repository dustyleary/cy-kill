import re

def parse(text):
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

