# this is copied wholesale from ninja-engine.
# arguably should be a library module - something that just provides
# some sane defaults for the broker.
require 'rzmq_brokers'
module Ninja
class ZmqChannel
  def initialize(master_context, broker, log_transport, channel, reactor)
    req_method = method(:handle_request)
    dis_method = method(:handle_disconnect)
    
    @reactor = reactor
    @channel = channel
    @actions = {}
    @blocks = {}  # will be empty for software channels
    name = @channel.class.to_s.split('::').last.downcase
    

    
    @worker_config = RzmqBrokers::Worker::Configuration.new do
      name "worker-#{name}"
      exception_handler nil
      poll_interval 250
      context master_context
      log_endpoint log_transport

      endpoint broker
      connect  true
      service_name  name
      heartbeat_interval 3_000
      heartbeat_retries 3
      on_request  req_method
      on_disconnect dis_method

      base_msg_klass RzmqBrokers::Majordomo::Messages
    end

    success = method(:client_success)
    failure = method(:client_failure)
    @client_config = RzmqBrokers::Client::Configuration.new do
      context master_context
      log_endpoint log_transport

      endpoint  broker
      connect  true
      max_broker_timeouts 1

      on_success  success
      on_failure  failure

      base_msg_klass RzmqBrokers::Majordomo::Messages
    end
    
  end
  
  def run
    puts "run"
    @worker = RzmqBrokers::Worker::Worker.new(@worker_config)
    @client = RzmqBrokers::Client::Client.new(@client_config)
    @channel.set_client(@client) if @channel.respond_to? :set_client
  end

  def client_success(message)
    puts "successful client: #{message.inspect}"
  end
  
  def client_failure(message)
    puts "unsuccessful client: #{message.inspect}"
  end

  def add_action(request,response)
    methodname = "add_action_#{request.name}"
    @actions[request.rule_id] = {
      :static_addins => request.data,
      :auth => request.auth
    }
    
    # optional initialiser
    @channel.send methodname, request.data, request.rule_id if @channel.respond_to? methodname
    @worker.succeeded response.sequence_id, response.encode
  end

  def remove_action(request,response)
    @channel.actions.delete request.rule_id
    # optional deinitialiser
    channel.send methodname, @reactor, request.data, request.rule_id if @channel.respond_to? methodname
    response.answer "ok"
    @worker.succeeded response.sequence_id, response.encode
  end
  
  
  def add_trigger(request,response)
    methodname = "add_trigger_#{request.name}"
    safely(response) do
      if @channel.respond_to? methodname
        @reactor.next_tick do
          # puts @reactor.inspect
          # puts request.data.inspect
          puts methodname
          # this bears explanation
          # the block we pass in is what is executed when the trigger
          # is actually executed - it is generally _not_ executed at
          # the time we call the channel.
          @channel.send methodname, @reactor, request.data, request.rule_id do |trigger_data={}|
            # puts request.inspect
            target_channel = request.data.fetch 'service'
            target_action = request.data.fetch 'action'
            rule_id = request.rule_id
            # static_addins = data.fetch 'static_addins'
            req = NinjaBlocks::LookupRequest.new do
              service_name target_channel
              rule_id rule_id
              message_type "do"
              entity_type "action"
              name target_action
              data trigger_data
            end
            # should we be caring about the response?
            @client.process_request req
          end
        end
        response.answer "ok"

          
        @worker.succeeded response.sequence_id, response.encode
      end
    end
  end
  
  def do_action(request, response)
    puts "received #{request.inspect}"
    methodname = "do_action_#{request.name}"
    
    if ! @channel.respond_to? methodname
      puts "#{@channel} can't handle #{methodname}"
      result = nil
    else
      result = @channel.send methodname, request.data
    end
    
    response.answer result
    # we say that it succeeded, because at this level it did.
    @worker.succeeded response.sequence_id, response.encode
  end

  # add a ninja to this channel
  def add_ninja(request, response)
    
    @blocks[[request.data.fetch("block_id"),
             request.data.fetch("line_id")]] = {} # may want to stash
                                                 # something later
    safely(response) do
      @channel.add_ninja request.data if @channel.respond_to? :add_ninja
      @worker.succeeded response.sequence_id, response.encode
    end
  end

  def remove_ninja(request, response)

    safely(response) do
      @channel.remove_ninja request.data if @channel.respond_to? :remove_ninja
      @worker.succeeded response.sequence_id, response.encode
    end.tap do |x|
      @blocks.delete request.rule_id
    end
      
  end

  def handle_request(worker, message)
    puts "request"
    # work out what kind of request this is.
    request = NinjaBlocks::Message.create_from(worker, message)
    response = NinjaBlocks::LookupReplySuccess.from_request(request)
    raise "don't understand message type #{request.message_type}" unless
      %w{add  remove do}.include? request.message_type
    raise "don't understand entity type #{request.entity_type}" unless
      %w{trigger action ninja}.include? request.entity_type
    begin
      self.send "#{request.message_type}_#{request.entity_type}", request, response
    rescue => e
      puts e.inspect
      puts e.backtrace
    end
     
  end

  def handle_disconnect(message)
    STDERR.puts "how did I get here?"
    exit!
  end

  protected
  def safely(response)
    yield
  rescue => e
    puts e.inspect
    puts e.backtrace
    puts @actions.inspect
    puts request.rule_id
    
    puts "hope that's enough. weird, no?"
    
    @worker.failed response.sequence_id, e.to_s    
  end
  

  
end # Channel
end

