require 'rubygems'
require 'ffi-rzmq'
require 'zmqmachine'
require 'json'





class SerialListener
  attr_reader :topics
  def initialize context, ports, client, topic = nil
    
    @context = context
    @ports = ports
    @client = client
    (@topics ||= []) << topic.to_s
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

  def add_trigger(line, rule_id, channel, action)
    
  end

  def remove_trigger(line, rule_id)

  end
  
  def on_readable socket, messages
    messages.each do |m|
      begin
        data = JSON.parse m.copy_out_string
        #      puts data.inspect
        # puts @cached.inspect
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
      end
      rescue => e
        puts "got an error: #{e}"
      end 
    end
  end

end
