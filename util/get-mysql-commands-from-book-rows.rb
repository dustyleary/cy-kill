#!/usr/bin/env ruby

require 'rubygems'

require 'json'
require 'pp'

def showUsage
  $stderr.puts "USAGE: #{$0} < INPUT"
  exit 1
end

showUsage unless ARGV.length == 0

enum = $stdin.each

$created_tables = {}

def rowInsertText row
  return %Q[("#{row['prePattern']}","#{row['postPattern']}",#{row['result']=='win' && '1' || '0'},"#{row['filename']}")]
end

def emit_mysql_commands batch
  json_rows = batch.map { |t| JSON.parse t }
  by_patternType = json_rows.group_by { |r| r['patternType'] }
  by_patternType.each { |patternType, rows|
    if not $created_tables[patternType]
      puts "CREATE TABLE IF NOT EXISTS #{patternType} (
        id INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
        prePattern CHAR(#{rows[0]['prePattern'].length}),
        postPattern CHAR(#{rows[0]['postPattern'].length}),
        win INT(1), move INT,
        filename CHAR(32),
        KEY prePattern (prePattern)
        ) Engine=MyISAM;"
      $created_tables[patternType] = 1
    end
    puts "INSERT INTO #{patternType} (prePattern, postPattern, win, filename) VALUES\n#{rows.map{|r|rowInsertText(r)}.join",\n"};"
  }
end

while true
  batch = enum.take 10000
  emit_mysql_commands batch
  break if batch.empty?
end
