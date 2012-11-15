#!/usr/bin/env ruby

require 'rubygems'
require 'pp'
require 'sqlite3'
require 'murmurhash3'

def showUsage
  $stderr.puts "USAGE: #{$0} <sqlite_file> < INPUT"
  exit 1
end

#showUsage unless ARGV.length == 1
#
#$db = SQLite3::Database.new ARGV[0]
#$table_exists = {}
#puts 'SQLite version: ' + $db.get_first_value('SELECT SQLITE_VERSION()')
#
#def insert_pattern patternType, prePattern, postPattern
#  ints = MurmurHash3::Native128.murmur3_128_str_hash prePattern
#  prePatternHash = ints.pack('L*')
#  postPatternBinary = postPattern[1..-1].split(':').map { |x| x.to_i(16) }.pack('L*')
#
#  preBlob = SQLite3::Blob.new(prePatternHash)
#  postBlob = SQLite3::Blob.new(postPatternBinary)
#
#  $db.execute("CREATE TABLE IF NOT EXISTS #{patternType}(prePattern BLOB, postPattern BLOB, num INTEGER, PRIMARY KEY (prePattern, postPattern))") unless $table_exists[patternType]
#  $table_exists[patternType] = 1
#  $db.execute("INSERT OR IGNORE INTO #{patternType} (prePattern, postPattern, num) VALUES (?,?,0)", preBlob, postBlob)
#  $db.execute("UPDATE #{patternType} SET num=num+1 WHERE prePattern=? AND postPattern=?", preBlob, postBlob)
#end

total_files = 0
prePatterns = {}
preKey = nil
postKey = nil
patternType = nil

#$db.execute("BEGIN")

def insert_pattern patternType, prePattern, postPattern
  puts "#{patternType} #{prePattern} #{postPattern}"
end

$stdin.each_with_index { |inputLine, i|
  inputLine.strip!
  next if inputLine.empty? or inputLine == '='

  m = /^= file (.*)/.match inputLine
  if m
    total_files += 1
    if 0 == total_files % 10
      #$db.execute("COMMIT")
      #$db.execute("BEGIN")
      $stderr.puts "file #{total_files} #{m[1]}"
    end
    raise RuntimeError, "new file without finishing up previous patterns" unless prePatterns.empty?
    next
  end

  m = /^= good_player_move pattern_pre (\w*) (\w*)/.match inputLine
  if m
    preKey = "#{m[1]}-#{m[2]}"
    postKey = nil
    next
  end

  m = /^= good_player_move pattern_post (\w*) (\w*)/.match inputLine
  if m
    preKey = nil
    postKey = "#{m[1]}-#{m[2]}"
    patternType = m[2]
    raise RuntimeError, "postKey #{postKey} without existing prePattern" unless prePatterns[postKey]
    next
  end

  m = /^= PATTERN_AT_RESULT: (.*)/.match inputLine
  if m
    if preKey
      prePatterns[preKey] = m[1]
    elsif postKey
      raise RuntimeError, "postKey #{postKey} without existing prePattern" unless prePatterns[postKey]
      insert_pattern patternType, prePatterns[postKey], m[1]
      prePatterns.delete postKey
      postKey = nil
    else
      raise RuntimeError, "did not receive key before pattern #{m[1]}"
    end
    next
  end

  raise RuntimeError, "could not handle line: #{inputLine}"
}

