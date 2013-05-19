#!/usr/bin/env ruby

require 'rubygems'
require 'pp'
require 'sgf'
require 'stringio'

parser = SGF::Parser.new

def showUsage
  $stderr.puts "USAGE: #{$0} <root_dir>"
  exit 1
end

showUsage unless ARGV.length == 1

if not File.directory? ARGV[0]
  $stderr.puts "ERROR: directory not found: #{ARGV[0]}"
  exit 1
end

def good_rank rank
  return false if rank.end_with? 'k'
  return true if rank.end_with? 'p'
  return true if rank[0...-1].to_i >= 5
  return false
end

A = "A"[0].ord
I = "I"[0].ord
def sgfToCoords pt
  x = pt.upcase[0].ord-A
  y = pt.upcase[1].ord-A
  return [x,y]
end

def coordsToGtp coords
  x,y = coords
  x+=1 if x >= (I-A)
  (A+x).chr + (y+1).to_s
end

$gMove = 0

files = Dir["#{ARGV[0]}/**/*.sgf"]
#files = files[0...1]
files.each do |file|
  collection = parser.parse File.read(file)
  game = collection.games[0]

  filename = File.basename(file)

  rank = {}
  rank['B']  = game.black_rank rescue next
  rank['W']  = game.white_rank rescue next

  good_player = {'B' => good_rank(rank['B']), 'W' => good_rank(rank['W'])}

  if not (good_player['B'] or good_player['W'])
    $stderr.puts "skipping file #{file}:  bad ranks #{rank['B']} #{rank['W']}"
    next
  end

  puts "echo_text file #{file}"
  puts "clear_board"

  ab = game.first.properties['AB'] || []
  aw = game.first.properties['AW'] || []

  ab.each { |pt| puts "play B #{coordsToGtp sgfToCoords(pt)}" }
  aw.each { |pt| puts "play W #{coordsToGtp sgfToCoords(pt)}" }

  moves = game.drop(1)
  moves.each_with_index do |node, moveNum|
    p = node.properties
    color = 'B' if p['B']
    color = 'W' if p['W']
    if p[color].empty?
      puts "play #{color} pass"
      next
    end

    coords = sgfToCoords p[color]
    gtpPoint = coordsToGtp coords

    if good_player[color]
      $gMove += 1

      puts "echo_text good_player_local_move moveId=#{$gMove} rank=#{rank[color]} gtpPoint=#{gtpPoint} filename=#{filename}"

      patterns_to_grab = {}
      #[9,3].each { |sz|
      [3].each { |sz|
        puts "valid_move_patterns #{sz} #{color}"
      }

      puts "play #{color} #{gtpPoint}"

    else
      puts "play #{color} #{gtpPoint}"
    end
  end
end

