require 'rubygems'
require 'ffi-rzmq'
require 'zmqmachine'
require 'json'





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
      begin
        data = JSON.parse m.copy_out_string
        data['ports'].each do |chunk|
          k = chunk['port'].to_i
          type = chunk['type']
          if line = @lines[k]
            line.check_for_portchange(type) do
              @client.handle_portchange(k,type)
            end
            line.update(chunk)
          end
        end
      rescue => e
        # let's clean the arduino stream up soon, hm?
        # puts "got an error: #{e}"
      end 
    end
  end

end
