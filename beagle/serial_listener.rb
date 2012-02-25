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
    @ninja_ports = {}
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
        value = chunk["value"]
        type = chunk['type']
        if @ninja_ports[k] != type
          # we've had a port change: either a plugin has appeared or disappeared
          # do something useful
          puts "port #{k} = #{type}"
          @ninja_ports[k] = type
          client.handle_portchange(k, type)
        end
        # if @lines[k] isn't set, we want to report changes, but not send events
        next unless @lines[k]
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
