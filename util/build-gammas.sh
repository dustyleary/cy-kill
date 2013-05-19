#!/bin/bash -e

./build-mm-input.rb | ~/dev/mm/mm
./parse-mm-results.rb | mysql -uroot gobook

