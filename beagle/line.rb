# Weird as it seems, each line may maintain many triggers.
# this ranges from sensible (below 10 degrees, turn on heaters - above
# 50, turn on aircon) to dubious (one rule for when a button is
# plugged in to line x, another for when a humidity sensor is), but
# the overriding goal is not to surprise the user.

class Line
  LIMIT = 2
  def initialize(line_id)
    @line_id = line_id.to_i
    @monitors = []
    @changed = 0
    @accessory = nil
    @new_accessory = nil
  end
  
  def add_monitor(monitor)
    @monitors << monitor
  end

  def remove_monitor(rule_id)
    @monitors.delete{|trigger| trigger.rule_id == rule_id}
  end

  def portchanged?(accessory_type)
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
        puts "monitors: #{@monitors.inspect}"
        @monitors.each do |trigger|
          trigger.current_accessory = @accessory
        end
        puts "changed from #{orig} to #{@accessory}!"
        return true
      end
    end
    return false

  end
  
  def update(chunk,client)
    
    @monitors.reject! do |monitor|
      value = chunk["value"]
      type = chunk['type'].downcase # should this matter at this point?
      extras = { :line => @line_id, :port_type => @accessory}
      if type == monitor.channel
        monitor.last?(value) do |message|
          message.data.merge!(extras)
          client.process_request message
        end
      else
        puts "#{monitor.channel} registered, but #{type} attached"
        false
      end
    end
  end
  
end
