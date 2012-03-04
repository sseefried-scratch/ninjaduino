load 'ninja/zmq_channel.rb'
load 'ninja/driver.rb'
load 'camera.rb'
Ninja::Driver.new(Camera.new({})).go

