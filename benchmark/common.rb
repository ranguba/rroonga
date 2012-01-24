require 'benchmark'
require 'tempfile'

def memory_usage()
  status = `cat /proc/#{$$}/status`
  lines = status.split("\n")
  lines.each do |line|
    if line =~ /^VmRSS:/
      line.gsub!(/.*:\s*(\d+).*/, '\1')
      return line.to_i / 1024.0
    end
  end
  return -1;
end

@items = []

def item(label, &block)
  @items << [label, block]
end

def report(index=0)
  width = @items.collect do |label, _|
    label.length
  end.max

  label, block = @items[index]
  if label.nil?
    puts "unavailable report ID: #{index}"
    puts "available IDs:"
    @items.each_with_index do |(label, block), i|
      puts "#{i}: #{label}"
    end
    exit 1
  end

  if index.zero?
    puts(" " * (width - 1) + Benchmark::Tms::CAPTION.rstrip + "memory".rjust(11))
  end
  # GC.disable
  before = memory_usage
  result = Benchmark.measure(&block)
  # GC.enable
  GC.start
  size = memory_usage - before

  formatted_size = "%7.3f" % size
  puts "#{label.ljust(width)} #{result.to_s.strip} #{formatted_size}MB"
end
