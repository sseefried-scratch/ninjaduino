# Weird as it seems, each line may maintain many triggers.
# this ranges from sensible (below 10 degrees, turn on heaters - above
# 50, turn on aircon) to dubious (one rule for when a button is
# plugged in to line x, another for when a humidity sensor is), but
# the overriding goal is not to surprise the user.

class Line
  LIMIT = 2
  def initialise(line_id)
    @line_id = line_id
    @triggers = {}
    @changed = 0
    @accessory = nil
    @new_accessory = nil
  end
  
  def add_trigger(rule_id, trigger)
    @triggers[rule_id] = trigger
  end

  def remove_trigger(rule_id)
    @triggers.delete rule_id
  end

  def check_for_portchange(accessory_type)
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
        @triggers.each do |k, trigger|
          trigger.enabled = trigger.channel == @accessory
        end
      end
    end
  end
  
  def update(chunk)
    @triggers.each do |rule_id, trigger|
      value = chunk["value"]
      type = chunk['type'] # should this matter at this point?
      if type == trigger.channel
        @client.handle_serial chunk.merge({:line_id => @line_id})  if trigger.fire?(value)
      else
        if trigger.enabled
          # our logic is broken if we get here.
          raise "port type mismatch: #{type} is not #{trigger.type}"
        else
          ## exactly as expected - it's an old trigger that will
          ## spring back to life when its original accessory is
          ## plugged back in.
        end
      end
    end
  end
  
end
