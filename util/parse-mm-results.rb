#!/usr/bin/env ruby

puts "DROP TABLE IF EXISTS pat3_gammas;"
puts "CREATE TABLE pat3_gammas (pat3 char(9) not null primary key, gamma double) engine=myisam;"

$stdin.each { |line|
    m = /(:\S+)\s*(\S+)/.match line
    if m
        puts "INSERT INTO pat3_gammas SET pat3='#{m[1]}', gamma='#{m[2]}';"
    end
}

