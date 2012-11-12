import os
import sys

is_win32 = 'win32' in sys.platform

def get_num_cpus():
    #http://codeliberates.blogspot.com/2008/05/detecting-cpuscores-in-python.html
    if hasattr(os, 'sysconf'):
         if os.sysconf_names.has_key("SC_NPROCESSORS_ONLN"):
             # Linux & Unix:
             ncpus = os.sysconf("SC_NPROCESSORS_ONLN")
             if isinstance(ncpus, int) and ncpus > 0:
                 return ncpus
         else:
             # OSX:
             return int(os.popen2("sysctl -n hw.ncpu")[1].read())
    if os.environ.has_key("NUMBER_OF_PROCESSORS"):
         ncpus = int(os.environ["NUMBER_OF_PROCESSORS"]);
         if ncpus > 0:
             return ncpus
    return 1

num_jobs = get_num_cpus() + 1
SetOption('num_jobs', num_jobs)
print "running with -j", GetOption('num_jobs')

build_debug = int(ARGUMENTS.get('debug', 0))
build_tests = int(ARGUMENTS.get('tests', 0))

def BaseEnv():
    env = Environment(
        MSVC_VERSION='9.0',
        MSVC_BATCH=True
    )
    env.Append(
        CCFLAGS=['-DHAVE_SSE2=1', '-DMEXP=19937'],
        CPPPATH=[
            '#/ext/googletest',
            '#/ext/googletest/include',
            '#/ext/mt',
            '#/ext/sqlite'
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
            LIBS=['pthread','dl']
        )
    return env

def Release(env):
    print "building Release"
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
    print "building Debug"
    env.Append(
        CPPDEFINES=['DEBUG'],
    )
    if is_win32:
        env.Append(
            CCFLAGS=['/Od'],
        )

env = BaseEnv()

if build_debug:
    Debug(env)
else:
    Release(env)

common_files = [
    '#/ext/mt/SFMT.c',
    '#/ext/sqlite/sqlite3.c',
    'src/Gtp.cpp',
    'src/GtpCyKill.cpp',
    'src/Pattern.cpp',
    'src/Zobrist.cpp',
]

test_files = [
    '#/ext/googletest/src/gtest-all.cc',
    '#/ext/googletest/src/gtest_main.cc',

    'src/test-NatSet.cpp',
    'src/test-Board.cpp',
    'src/test-Gtp.cpp',
    'src/test-WeightedRandomChooser.cpp',

    'src/test-TicTacToeGame.cpp',
]

def prog(n):
    env.Program(target=n, source=["src/%s.cpp"%n] + common_files)

prog('main-BenchmarkPlayouts')
prog('main-Connect4')
prog('main-CyKill')
prog('main-TicTacToe')

if build_tests:
    print "building tests"
    env.Program(target='test', source=test_files + common_files)

