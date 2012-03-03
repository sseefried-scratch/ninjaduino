class Monitor
  attr_reader :channel
  
  def initialize(timeout)
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
    return true if  @deadline < Time.now
    # send an update
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
  end
  
end
