require 'rubygems'
require 'ffi-rzmq'
require 'zmqmachine'
require 'json'


class SerialListener
  attr_reader :topics
  def initialize context, ports, client, topic = nil
    @switch_lo = 10
    @switch_hi = 1000
    @state = 0
    @context = context
    @received_count = 0
    @ports = ports
    @client = client
    @cached = {}
    (@topics ||= []) << topic.to_s
    @lines = {}
  end

  def activate_line(id)
    @lines[id] = true
  end

  def deactivate_line(id)
    @lines[id] = false
  end


  def on_attach socket
    puts "attached serial listener"    
    @ports.each do |port|
      address = ZM::Address.new '127.0.0.1', port, :tcp
      rc = socket.connect address

      @topics.each do |topic|
        puts "subscribe to [#{topic}]"
        socket.subscribe topic
      end
    end
  end

  def on_readable socket, messages
    messages.each do |m|
      begin
      data = JSON.parse m.copy_out_string
#      puts data.inspect
      # puts @cached.inspect
      data['ports'].each do |chunk|
        k = chunk['port']
        @cached[k] ||= 0 
        next unless @lines[k]
        value = chunk["value"]
        if @cached[k] == 0 && value > @switch_hi
          @cached[k] = 1
        elsif (@cached[k] == 1 && value < @switch_lo)
          @cached[k]=0
          # send button up event
          @client.handle_serial chunk.merge({:line_id => k})      
        else
	  next
        end
      end
      rescue => e
#        puts "got an error: #{e}"
      end 
    end
  end

end
