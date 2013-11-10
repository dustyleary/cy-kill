set -ex

mysql -uroot gobook < create-boardlocal-table.sql

./get-boardlocal-commands-from-sgfs.rb ttt | ../main-CyKill 2>/dev/null | ./get-boardlocal-book-rows-from-transcript.rb | ./get-mysql-commands-from-book-rows.rb | mysql -uroot gobook

# FIXME: 'INSERT IGNORE' because of...  mysql?

mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM t19 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM t17 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM t15 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM t13 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM t11 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM t9 GROUP BY prePattern, postPattern'

mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM c19 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM c17 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM c15 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM c13 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM c11 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM c9 GROUP BY prePattern, postPattern'

mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM e19 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM e17 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM e15 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM e13 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM e11 GROUP BY prePattern, postPattern'
mysql -uroot gobook -e 'INSERT IGNORE INTO boardlocal SELECT prePattern, postPattern, COUNT(*) AS num FROM e9 GROUP BY prePattern, postPattern'

