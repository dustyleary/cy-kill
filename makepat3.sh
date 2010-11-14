#!/bin/sh
python extract_patterns.py  ../training 3 |  python compute_pattern_elos.py pat3.dat
