# this is the program the ninja block itself runs.


require 'uuid'
# load 'ninja_message.rb'
require 'ninja_blocks'
load 'serial_listener.rb'
require 'rzmq_brokers'

class CloudListener
  def initialize(master_context, broker, log_transport, reactor, identity)

    @lines = []
    req_method = method(:handle_request)
    dis_method = method(:handle_disconnect)
    @identity = identity
    @master_context = master_context 
    @reactor = reactor

    @worker_config = RzmqBrokers::Worker::Configuration.new do
      name identity
      exception_handler nil
      poll_interval 250
      context master_context
      log_endpoint log_transport

      endpoint broker
      connect  true
      service_name  identity
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
    
    puts "configured"
  end

  def run
    puts "run"
    @worker = RzmqBrokers::Worker::Worker.new(@worker_config)
    @client = RzmqBrokers::Client::Client.new(@client_config)
    ctx = ZM::Reactor.new.run do |context|
      @serial = SerialListener.new(context, [5556], self, '')
      context.sub_socket @serial    
      @serial.activate_line(2) # hack
    end
  end

  def client_success(message)
    puts "successful client: #{message.inspect}"
  end
  
  def client_failure(message)
    puts "unsuccessful client: #{message.inspect}"
  end

  def remove_trigger(request, response)
    safely(response) do
      @lines[request.data.fetch('line').to_i]=nil
    end
  end
  

  def add_trigger(request, response)
    safely(response) do
      # stop ignoring stuff coming in
      @lines[request.data.fetch('line').to_i] = {
        :rule_id => request.rule_id,
        :channel => request.data.fetch('service'),
        :action => request.data.fetch('action')
      }
      @worker.succeeded response.sequence_id, response.encode
    end
  end
  
  def handle_request(worker, message)
    puts "request"
    # work out what kind of request this is.
    request = NinjaBlocks::Message.create_from(worker, message)
    response = NinjaBlocks::LookupReplySuccess.from_request(request)
    safely(response) do
      raise "don't understand message type #{request.message_type}" unless
        %w{add  remove do}.include? request.message_type
      raise "don't understand entity type #{request.entity_type}" unless
        %w{trigger action}.include? request.entity_type
      self.send "#{request.message_type}_#{request.entity_type}", request, NinjaBlocks::LookupReplySuccess.from_request(request)
    end
  end

  def handle_serial(data)
    puts "got #{data.inspect}"    
    puts @lines.inspect

    if !(target = @lines[data["port"]])
      puts "ignoring incoming on #{data['port']}"
    else
      service = target[:channel]
      rule_id = target[:rule_id]  
      action = target[:action]  
        rule_id rule_id
        message_type "do"
        entity_type "action"
        # hm, we don't really know this
        name action
        data({ :time => Time.now.to_s })
      end
      @client.process_request req
    end
  end

  def handle_portchange(line_id, port_type)
    puts "handling a portchange"
    identity = @identity
    begin 
    req = NinjaBlocks::LookupRequest.new do
      service_name "port_watcher"
      rule_id 0 # utter bullshit
      message_type "do"
      entity_type "action"
      name "port_event"
      data({ :port_type => port_type.downcase,
             :block => identity,
             :line => line_id
           })
    
    end
    puts "sending #{req.inspect}"
    @client.process_request req
    rescue => e
      puts e.inspect
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
    
    puts "hope that's enough. weird, no?"
    
    @worker.failed response.sequence_id, e.to_s    
  end
  

  
end # Channel

