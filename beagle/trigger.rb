# so our basic model here is that every sensor is
# associated with a list of triggers. each can fire independently
# and maintain its own state.

class Trigger
  def initialize(reset_level, trigger_level, transformer)
    # if reset_level is higher than the trigger level, then
    # that means we're approaching the trigger from above - ie,
    # trigger if temperature goes _below_ X.
    
    @fired = false
    @comparator = reset_level > trigger_level ? lambda {|x,y| x < y}
                                              : lambda {|x,y| x > y}

    @reset_level= reset_level
    @trigger_level = trigger_level
    @transformer = transformer
  end

  def fire?(val)
    
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
