#!/usr/bin/env ruby

require 'rubygems'
require 'json'
require 'pp'

def showUsage
  $stderr.puts "USAGE: #{$0} < INPUT"
  exit 1
end

showUsage unless ARGV.length == 0

total_files = 0
moveInfos = {}
curMove = nil

def insert_pattern pattern
  puts pattern.to_json
end

def unpack_good_player_move m
  d = {
    :moveId => m[1],
    :rank => m[2],
    :gtpPoint => m[3],
    :filename => m[4],
  }
end

$stdin.each_with_index { |inputLine, i|
  inputLine.strip!
  next if inputLine.empty? or inputLine == '='

  m = /^= file (.*)/.match inputLine
  if m
    total_files += 1
    if 0 == total_files % 10
      $stderr.puts "file #{total_files} #{m[1]}"
    end
    next
  end

  m = /^= good_player_local_move moveId=(\S*) rank=(\S*) gtpPoint=(\S*) filename=(\S*)$/.match inputLine
  if m
    d = unpack_good_player_move m
    curMove = d
    next
  end

  m = /^= VALID_MOVE_PATTERNS (\S*) (.*)$/.match inputLine
  if m
    puts "INSERT INTO movelocal_moves SET size='#{m[1]}', winner='#{curMove[:gtpPoint]}', moves='#{m[2]}';"
    next
  end

  raise RuntimeError, "could not handle line: #{inputLine}"
}

