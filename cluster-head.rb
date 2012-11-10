#!/usr/bin/env ruby

HeaderCommandNames = %W[boardsize clear_board komi play engine_param]
AllCommands = HeaderCommandNames + %W[name protocol_version version quit]
remote_cmds = File.new(ARGV[0], 'r').readlines.map(&:chomp).reject {|l| l=='' }

STDERR.puts "# `remote_cmds: #{remote_cmds.inspect}"

boardsizeCommand = "boardsize 19"

defaultCommands = %Q[
engine_param max_playouts 200000
engine_param max_think_millis 3600000000
engine_param uct_kCountdownToCertainty 100000000
engine_param uct_kNumPlayoutsPerTrace 5
engine_param uct_kRaveEquivalentPlayouts 200
engine_param uct_kTracesPerGuiUpdate 20000
komi 0.5
clear_board
].chomp.split "\n"
headerCommands = [boardsizeCommand] + defaultCommands

def gtpSuccess r=nil
  if r
    puts "= #{r}\n\n"
  else
    puts "=\n\n"
  end
end

def gtpFailure r
  return puts "? #{r}\n\n"
end

STDIN.sync = true
STDOUT.sync = true

def tmpFilename basename
  i = 0
  for i in (0...10000)
    if basename.include? '/'
      n = "#{basename}-#{i}"
    else
      n = "/tmp/#{Process.pid}-#{basename}-#{i}"
    end
    if not File.exists? n
      return n
    end
  end
end

while true
  fullcmd = STDIN.gets.chomp
  STDERR.puts "#>> #{fullcmd}"
  next if fullcmd == ''
  cmd = fullcmd.split(' ')[0]
  if cmd == 'clear_board'
    headerCommands = [boardsizeCommand] + defaultCommands
  end
  if HeaderCommandNames.include? cmd
    headerCommands << fullcmd
    STDERR.puts "#>> headerCommands: #{headerCommands.inspect}"
    boardsizeCommand = fullcmd if cmd == 'boardsize'
    gtpSuccess
  else
    case cmd
    when 'name' then gtpSuccess 'cy-kill cluster'
    when 'protocol_version' then gtpSuccess '2'
    when 'version' then gtpSuccess '0.1'
    when 'quit' then exit 0
    when 'list_commands' then gtpSuccess AllCommands.join "\n"
    when 'genmove'
      tmpDirName = tmpFilename 'genmove'
      Dir.mkdir tmpDirName

      pids = []
      ofns = []
      remote_cmds.each_with_index do |cmd,i|
        fn = tmpFilename "#{tmpDirName}/in"
        lines = [] + headerCommands
        lines << "engine_param uct_kModuloPlayoutsNumerator #{i}"
        lines << "engine_param uct_kModuloPlayoutsDenominator #{remote_cmds.length}"
        lines << fullcmd
        contents = lines.join "\n"
        f = File.open(fn, 'w') {|f|
          f.puts contents
        }
        ofn = tmpFilename "#{tmpDirName}/out"
        ofns << ofn
        File.open(ofn, 'w') {|f| }
        pid = fork {
          exec "#{cmd} < #{fn} > #{ofn} 2>&1"
        }
        pids << pid
      end
      t = Time.new
      STDERR.puts "# waiting on #{pids.length} child processes"
      Process.waitall.each { |pid,status|
        abort "error in pid #{pid}" unless status.success?
      }
      sleep 1
      STDERR.puts "# move took %.2fs" % (Time.new-t)
      all_lines = ofns.map { |ofn| File.new(ofn, 'r').readlines.map &:chomp }.flatten
      STDERR.puts "#lines: #{all_lines.count}"
      all_lines.reject! { |l| not l.include? 'candidate' }
      STDERR.puts "#lines with candidate: #{all_lines.count}"
      moves = all_lines.map {|l|
        m = /move candidate: (\w+).*value: ([0-9.]+)/.match l
        [m[2].to_f, m[1], l]
      }
      moves = moves.sort.reverse
      moves.each { |_,_,statusline|
        STDERR.puts statusline
      }
      STDERR.puts "# move took %.2fs" % (Time.new-t)
      color = /genmove (\w+)/.match(fullcmd)[1]
      headerCommands << "play #{color} #{moves[0][1]}"
      gtpSuccess moves[0][1]
    else
      gtpFailure 'unknown command'
    end
  end
end

