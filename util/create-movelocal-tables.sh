mysql -uroot gobook < create-movelocal-table.sql

./get-movelocal-commands-from-sgfs.rb ttt | ../main-CyKill 2>/dev/null | ./get-movelocal-sql-from-transcript.rb | mysql -uroot gobook

