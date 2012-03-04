BROKER='tcp://au.ninjablocks.com:5773'
require 'ffi-rzmq'
require 'zmqmachine'
# load 'twitter_chan.rb'
module Ninja
  class Driver
  def initialize(channel)
    @master_context = ZMQ::Context.new
    @log_transport = "inproc://reactor_log"
    master_context=@master_context
    @logger_config = ZM::Configuration.new do
      context master_context
      name 'logger-server'
    end
    @channel = channel

  end


  def go
    reactor = @reactor
    master_context = @master_context
    log_transport = @log_transport
    @reactor = ZM::Reactor.new(@logger_config).run do |reactor|
      log_config = ZM::Server::Configuration.new do
        endpoint log_transport
        bind true
        topic ''
        context master_context
        reactor reactor
      end

      log_config.extra = {:file => STDOUT}

      log_server = ZM::LogServer.new(log_config)
    end
    # time for the log_server to spin up, as well as the broker
    sleep 5

    worker = ZmqChannel.new(@master_context, BROKER, @log_transport, @channel, @reactor)

    worker.run


    while true
      sleep 1
    end
  end
end
end

