#!/bin/sh
python extract_patterns.py  ../training 5 |  python compute_pattern_elos.py pat5.dat
