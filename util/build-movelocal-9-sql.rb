#!/usr/bin/env ruby

require 'rubygems'
require 'mysql'
require 'json'

$conn = Mysql::new('localhost', 'root', '', 'gobook')

def generate_rows
    cur_id = -1
    batch_size = 2
    Enumerator.new { |g|
        while true
            res = $conn.query "select id,winner,moves from movelocal_moves where size=9 and id>#{cur_id} limit #{batch_size}"
            while (row = res.fetch_hash())
                new_id = row['id']
                g.yield row
            end
            break if new_id == cur_id
            cur_id = new_id
        end
    }
end

puts "LOCK TABLES movelocal_patterns WRITE;"

generate_rows.each_with_index { |row,i|
    moves = JSON.parse row['moves']
    winning_pattern = moves[row['winner']]
    if i!=0 and 0 == i%10000
        $stderr.puts "#{Time.now} #{i} #{winning_pattern}"
        puts "UNLOCK TABLES;"
        puts "LOCK TABLES movelocal_patterns WRITE;"
    end
    puts "INSERT INTO movelocal_patterns (prepattern,num) VALUES ('#{winning_pattern}', 1) ON DUPLICATE KEY UPDATE num=num+1;"
}

