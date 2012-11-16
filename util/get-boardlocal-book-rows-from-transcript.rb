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
patterns = {}
preKey = nil
postKey = nil

def insert_pattern pattern
  puts pattern.to_json
end

def unpack_good_player_move m
  d = {
    :moveId => m[1],
    :patternType => m[2],
    :rank => m[3],
    :move => m[4],
    :result => m[5],
    :color => m[6],
    :gtpPoint => m[7],
    :filename => m[8],
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
    raise RuntimeError, "new file without finishing up previous patterns" unless patterns.empty?
    next
  end

  m = /^= good_player_move pattern_pre moveId=(\S*) patternType=(\S*) rank=(\S*) move=(\S*) result=(\S*) color=(\S*) gtpPoint=(\S*) filename=(\S*)$/.match inputLine
  if m
    d = unpack_good_player_move m
    preKey = "#{d[:patternType]}-#{d[:moveId]}"
    postKey = nil
    raise RuntimeError, "duplicate preKey" if patterns[preKey]
    patterns[preKey] = d
    next
  end

  m = /^= good_player_move pattern_post moveId=(\S*) patternType=(\S*) rank=(\S*) move=(\S*) result=(\S*) color=(\S*) gtpPoint=(\S*) filename=(\S*)$/.match inputLine
  if m
    d = unpack_good_player_move m
    preKey = nil
    postKey = "#{d[:patternType]}-#{d[:moveId]}"
    raise RuntimeError, "postKey #{postKey} without existing pattern" unless patterns[postKey]
    next
  end

  m = /^= PATTERN_AT_RESULT: (.*)/.match inputLine
  if m
    if preKey
      raise RuntimeError, "pattern for #{preKey} without existing pattern row" unless patterns[preKey]
      patterns[preKey][:prePattern] = m[1]
    elsif postKey
      raise RuntimeError, "pattern for #{postKey} without existing pattern row" unless patterns[postKey]
      patterns[postKey][:postPattern] = m[1]

      insert_pattern patterns[postKey]
      patterns.delete postKey
      postKey = nil
    else
      raise RuntimeError, "did not receive key before pattern #{m[1]}"
    end
    next
  end

  raise RuntimeError, "could not handle line: #{inputLine}"
}

