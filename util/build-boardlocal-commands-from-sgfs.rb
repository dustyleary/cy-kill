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

def closestStarCoordsAndType coords
  x,y = coords
  if x<=6 then x = 3 elsif x>=12 then x = 15 else x = 9 end
  if y<=6 then y = 3 elsif y>=12 then y = 15 else y = 9 end
  if x==9 and y==9
    t = 't'
  elsif x==y
    t = 'c'
  else
    t = 'e'
  end
  [[x,y], t]
end

$gMove = 0

files = Dir["#{ARGV[0]}/**/*.sgf"]
#files = files[0...10]
files.each do |file|
  collection = parser.parse File.read(file)
  game = collection.games[0]

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
  moves.each do |node|
    p = node.properties
    color = 'B' if p['B']
    color = 'W' if p['W']
    if p[color].empty?
      puts "play #{color} pass"
      next
    end

    coords = sgfToCoords p[color]
    gtpPoint = coordsToGtp coords
    starCoords, starType = closestStarCoordsAndType coords

    if good_player[color]
      $gMove += 1

      patterns_to_grab = {}
      [19,17,15,13,11,9].each { |sz|
        n = "#{starType}#{sz}"
        patterns_to_grab[n] = [sz, coordsToGtp(starCoords)]
      }
      patterns_to_grab['t19'] = [19, 'K10']

      patterns_to_grab.each { |n, info|
        sz, starPtGtp = info
        puts "echo_text good_player_move pattern_pre move#{$gMove} #{n} #{rank[color]} #{color} #{starPtGtp}"
        puts "pattern_at #{sz} #{color} #{starPtGtp}"
      }

      puts "play #{color} #{gtpPoint}"

      patterns_to_grab.each { |n, info|
        sz, starPtGtp = info
        puts "echo_text good_player_move pattern_post move#{$gMove} #{n} #{rank[color]} #{color} #{starPtGtp}"
        puts "pattern_at #{sz} #{color} #{starPtGtp}"
      }

    else
      puts "play #{color} #{gtpPoint}"
    end
  end
end

