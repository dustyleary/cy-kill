#!/usr/bin/env ruby

ENSURE_CLEAN_GIT = true
BOARD_SIZE = 9
GAMES = 100
GAMES = 2

def runcmd cmd
  puts cmd
  output = `#{cmd}`
  exit $?.exitstatus unless $?.success?
  return output
end

cykill_dir = File.expand_path File.join(File.expand_path($0), '..','..')
abort "Could not find cy-kill root dir" unless File.exists? File.join(cykill_dir, 'cy-kill.xcodeproj')

if ENSURE_CLEAN_GIT
  output = runcmd "cd #{cykill_dir} && git status --porcelain"
  lines = output.split "\n"
  p lines
  lines.each do |line|
    next if line == ''
    next if line == ' M util/do-gnugo-tournament.rb'
    next if line.include? "cykill-vs-gnugo"

    puts output
    puts "ERROR: there are unsaved changes in the git repo"
    exit 1
  end
end

rev = runcmd "cd #{cykill_dir} && git rev-parse HEAD"
rev = rev.strip

cykill_args = ['engine_param random_seed 42'] + ARGV
cykill_cmd = "#{cykill_dir}/cy-kill"
cykill_args.each do |a|
  cykill_cmd = %Q[#{cykill_cmd} "#{a}"]
end

outdir = "#{cykill_dir}/util/cykill-vs-gnugo-#{GAMES}games-#{BOARD_SIZE}x#{BOARD_SIZE}-#{rev}"
ARGV.each do |a|
  outdir = "#{outdir}-#{a.gsub /[^\w]/, ''}"
end
runcmd "mkdir -p #{outdir}"

cmd = %Q[gogui-twogtp -black 'gnugo --mode gtp' -games #{GAMES} -size #{BOARD_SIZE} -alternate -auto -sgffile #{outdir}/games -white '#{cykill_cmd}']
File.open(File.join(outdir, "cmdline"), 'w') { |f|
  f.puts cmd
}

runcmd cmd

