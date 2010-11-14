import os
import sys

is_win32 = 'win32' in sys.platform

num_cpu = max(1, int(os.environ.get('NUMBER_OF_PROCESSORS', 1)))
SetOption('num_jobs', num_cpu)
print "running with -j", GetOption('num_jobs')

def BaseEnv():
    env = Environment(
        MSVC_VERSION='9.0'
    )
    env.Append(
        CCFLAGS=['-DHAVE_SSE2=1', '-DMEXP=19937'],
        CPPPATH=[
            '#/ext/googletest',
            '#/ext/googletest/include',
            '#/ext/mt'
        ],
    )
    if is_win32:
        env.Append(
            LIBS=['winmm'],
            LINKFLAGS=['/DEBUG', '/LTCG'],
            CCFLAGS=['/EHsc', '/Zi'],
            CPPPATH=['#/ext/msvc-include'],
        )
    else:
        env.Append(
        )
    return env

def Release(env):
    env.Append(
        CPPDEFINES=['NDEBUG'],
    )
    if is_win32:
        env.Append(
            CCFLAGS=[
                '/MT', '/Ox', '/O2', '/Oi', '/Ot', '/Oy',
                '/fp:fast', '/Gr', '/GL',
                #'/Og', '/arch:SSE2', 
            ],
            LINKFLAGS=['/OPT:REF', '/OPT:ICF'],
        )
    else:
        env.Append(
            CCFLAGS=['-O3'],
        )

def Debug(env):
    env.Append(
        CPPDEFINES=['DEBUG'],
    )
    if is_win32:
        env.Append(
            CCFLAGS=['/Od'],
        )

env = BaseEnv()

if 0:
    Debug(env)
else:
    Release(env)

test_files = [
    '#/ext/googletest/src/gtest-all.cc',
    '#/ext/googletest/src/gtest_main.cc',

    '#/ext/mt/SFMT.c',

    'test_natset.cpp',
    'test_board.cpp',
    'test_gtp.cpp',
    'gtp.cpp',
]

env.Program(target='test', source=test_files)
env.Program(target='playout', source=['playout.cpp', '#/ext/mt/SFMT.c'])
env.Program(target='cy-kill', source=['cy-kill.cpp', 'gtp.cpp', '#/ext/mt/SFMT.c'])
