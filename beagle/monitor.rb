class Monitor
  attr_reader :channel
  PER_SECOND = 4
  def initialize(timeout)
    @next_send = Time.now + 1 / PER_SECOND
    @deadline = Time.now + timeout
    @channel = nil
  end

  def rule_id
    0 # ephemeral
  end

  def current_accessory=(accessory)
    if !@channel
      # first time: let's set it and go.
      @channel = accessory
    elsif @channel != accessory
      # might as well kill the updates, we're not plugged in any more.
      puts "killing the updates for #{@channel}, not plugged in any more"
      @deadline = Time.now
    else
      # business as usual!
    end
  end
  
  def last?(value)
    puts "last? for monitor #{self.inspect}"
    now = Time.now
    if @deadline < now
      puts "expiring monitor #{self.inspect}"
      return true if  @deadline < now
    end
    # send an update
    if @next_send < now
      puts "sending a monitor update: #{value}"
      req = NinjaBlocks::LookupRequest.new do
        service_name "port_watcher"
        rule_id 0
        message_type "do"
        entity_type "action"
        name "monitor"      
        data({ :value => value })
      end
      yield req
      return false
      @next_send = now
    end
  end
  
end
