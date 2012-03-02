# Weird as it seems, each line may maintain many triggers.
# this ranges from sensible (below 10 degrees, turn on heaters - above
# 50, turn on aircon) to dubious (one rule for when a button is
# plugged in to line x, another for when a humidity sensor is), but
# the overriding goal is not to surprise the user.

class Line
  LIMIT = 2
  def initialize(line_id, client)
    client = @client
    @line_id = line_id.to_i
    @monitors = []
    @changed = 0
    @accessory = nil
    @new_accessory = nil
    puts "Top: #{@triggers.inspect}"
  end
  
  def add_monitor(monitor)
    @triggers << monitor
  end

  def remove_monitor(rule_id)
    @triggers.delete{|trigger| trigger.rule_id == rule_id}
  end

  def portchanged?(accessory_type)
    puts "port changed for #{accessory_type}, #{@line_id}?"
    orig = @accessory
    if accessory_type != @accessory
      if accessory_type == @new_accessory
        @changed+=1
      else
        @new_accessory = accessory_type
        @changed = 1
      end
      if @changed > LIMIT
        @accessory = accessory_type
        @changed = 0
        @triggers.each do |trigger|
          trigger.current_accessory = @accessory
        end
        puts "changed from #{orig} to #{@accessory}!"
        return true
      end
    end
    return false

  end
  
  def update(chunk, client)
    
    @monitors.reject! do |monitor|
      value = chunk["value"]
      type = chunk['type'] # should this matter at this point?
      if type == monitor.channel
        monitor.last?(value) do |message|
          client.process_request message
        end
          
        # if monitor.fire?(value)
        #  @client.handle_serial chunk.merge({:line_id => @line_id})
          
      else
        if trigger.enabled
          # our logic is broken if we get here.
          raise "port type mismatch: #{type} is not #{trigger.type}"
        else
          ## exactly as expected - it's an old trigger that will
          ## spring back to life when its original accessory is
          ## plugged back in. keep it in the list, though.

          false
          
        end
      end
    end
  end
  
end
