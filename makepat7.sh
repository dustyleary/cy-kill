#!/bin/sh
python extract_patterns.py  ../training 7 |  python compute_pattern_elos.py pat7.dat
