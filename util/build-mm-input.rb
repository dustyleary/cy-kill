#!/usr/bin/env ruby

require 'rubygems'
require 'mysql'
require 'json'

conn = Mysql::new('localhost', 'root', '', 'gobook')
res = conn.query 'select winner,moves from movelocal_moves where size=3 limit 4000'

pattern_id = 0
pattern_ids = {}
pattern_ids_inverse = {}

games = []

did_row = false

row_index = 0

res.each_hash do |row|
    i = row_index
    row_index += 1
    $stderr.puts "got1 #{i}"
    moves = JSON.parse row['moves']
    moves.values.each { |pattern|
        if not pattern_ids[pattern]
            pattern_ids[pattern] = pattern_id
            pattern_ids_inverse[pattern_id] = pattern
            pattern_id += 1
        end
    }
    winning_pattern = moves[row['winner']]
    game = "#{pattern_ids[winning_pattern]}\n"
    game += moves.values.map { |pattern| "#{pattern_ids[pattern]}" }.join "\n"
    games << game
end

puts "! #{pattern_ids.size}"
puts "#{pattern_ids.size}"
pattern_id.times.each { |i|
    puts "1 #{pattern_ids_inverse[i]}"
}

puts "!"

games.each { |game|
    puts "#"
    puts game
}

