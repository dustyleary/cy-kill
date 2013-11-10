#!/usr/bin/env ruby

require 'json'

pattern_id_list = JSON.parse File.read('pattern_ids.out')

File.new('mm-with-freq.dat', 'r').each { |line|
    m = /(\S+)\s*(\S+)/.match line
    if m
      idx = m[1].to_i
      gamma = m[2]
      puts "#{pattern_id_list[idx]} #{gamma}"
    end
}

