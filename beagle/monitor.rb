class Monitor
  attr_reader :channel
  
  def initialize(timeout, channel)
    @deadline = Time.now + timeout
    @channel = channel
  end


  def current_accessory=(accessory)
    if @channel != accessory
      # might as well kill the updates, we're not plugged in any more.
      @deadline = Time.now
    end
  end
  
  def finished?(value)
    # send an update
    puts "sending an update for channel #{@channel}: #{value}"
    req = NinjaBlocks::LookupRequest.new do
      service_name "port_watcher"
      rule_id 0
      message_type "do"
      entity_type "action"
      action "monitor"
      data({ :value => value})
    end
    yield req
    return @deadline < Time.now
  end
  
end
