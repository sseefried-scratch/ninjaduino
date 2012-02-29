require 'rubygems'
require 'ffi-rzmq'
require 'zmqmachine'
require 'json'
load './line.rb'



class SerialListener
  attr_reader :topics
  def initialize context, ports, client, topic = nil
    @context = context
    @ports = ports # typically just one
    @client = client
    (@topics ||= []) << topic.to_s
    @lines = []
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

  def add_trigger(line, rule_id, channel_id, action)

    trigger = Trigger.new channel, action.data['reset_level'], action.data['trigger_level']
    (@lines[line] ||= Line.new line).add_trigger(rule_id,trigger)
    
  end

  def remove_trigger(line, rule_id)
    if line = @lines[line]
      line.remove_trigger(rule_id)
    end
  end
  
  def on_readable socket, messages
    messages.each do |m|
      data = nil
      begin
        data = JSON.parse m.copy_out_string
      rescue =>  e
      end
      next unless data
      puts "Data is data.inspect
      data['ports'].each do |chunk|
        k = chunk['port']
        type = chunk['type']
        puts chunk
        puts "#{k}:#{type}"
        line = (@lines[k] ||= Line.new k)
        @client.handle_portchange(k,type) if line.portchanged?(type)
        line.update(chunk)
      end 
    end
  end

end
