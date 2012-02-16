# this is the program the ninja block itself runs.
require 'zmqmachine'

load './cloud_listener.rb'

broker = "tcp://au.ninjablocks.com:5773"

master_context = ZMQ::Context.new
log_transport = "inproc://reactor_log"

logger_config = ZM::Configuration.new do
  context master_context
  name 'logger-server'
end

reactor = ZM::Reactor.new(logger_config).run do |reactor|
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

# fake id for the moment
identity = "n:1234"

# time for the log_server to spin up, as well as the broker
sleep 5

  cloud_listener = CloudListener.new(master_context, broker, log_transport,reactor, identity)
  cloud_listener.run
  #context.sub_socket @sub1_handler
while true
  puts "marking time"
  sleep 1
end


