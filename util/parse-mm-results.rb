#!/usr/bin/env ruby

require 'json'

puts "DROP TABLE IF EXISTS pat3_gammas;"
puts "CREATE TABLE pat3_gammas (pat3 char(9) not null primary key, gamma double) engine=myisam;"

pattern_id_list = JSON.parse File.read('pattern_ids.out')

File.new('mm-with-freq.dat', 'r').each { |line|
    m = /(\S+)\s*(\S+)/.match line
    if m
      idx = m[1].to_i
      gamma = m[2]
      puts "INSERT INTO pat3_gammas SET pat3='#{pattern_id_list[idx]}', gamma='#{gamma}';"
    end
}

