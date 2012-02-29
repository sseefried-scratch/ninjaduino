# so our basic model here is that every sensor is
# associated with a list of triggers. each can fire independently
# and maintain its own state.

class Trigger

  CHANNEL_DEFAULTS={
    "button" => { 'reset_level' => 10, 'trigger_level' => 1000 }
  }
  attr_reader :channel, :enabled
  def initialize(channel,
                 reset_level=nil,
                 trigger_level=nil) 

    # if reset_level is higher than the trigger level, then
    # that means we're approaching the trigger from above - ie,
    # trigger if temperature goes _below_ X.
    defaults = CHANNEL_DEFAULTS[channel] || {}
    @reset_level = reset_level || defaults['reset_level']
    @trigger_level = trigger_level || defaults['trigger_level']
    @transformer = defaults['transformer'] || lambda{|x|x}

    @channel = channel
    @enabled = true
    @fired = false
    @comparator = @reset_level > @trigger_level ? lambda {|x,y| x < y}
                                                : lambda {|x,y| x > y}

  end

  def enabled=(val)
    if val != @enabled
      if val == false
        # send a message somehow - we've been disabled, and the rule
        # should be marked as inactive FIXME
        warn "should be notifying server"
      end
    end
    @enabled = val
  end

  def fire?(value)
    
    if @fired
      if @comparator.call(@reset_level, value)
        @fired = false
      end
      false
    else
      if @comparator.call(value, @trigger_level)
        @fired = true
        true
      else
        false
      end
    end
  end
end
