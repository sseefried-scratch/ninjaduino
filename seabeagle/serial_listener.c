#include "mdworker.h"
// json?

#include "line.h"
#include "monitor.h"


struct serial_listener_t {
  int port; // just one now.
  char * identity;
  line * lines;
  mdcli_t * client;
};
// punting on  cloud, context.

/* this is connecting to local ports. Not 
void attach(serial_listener_t * serial) {
  
}
  def on_attach socket
    puts "attached serial listener"    
    @ports.each do |port|
      address = ZM::Address.new '127.0.0.1', port, :tcp
      rc = socket.connect address

      @topics.each do |topic|
        puts "subscribe to [#{topic}]"
        socket.subscribe topic
      end
    end
  end
  
  # send all the updates until we die
  # should probably be merged with add_trigger
  def add_monitor(line, monitor)
    # monitor the line, send to port_watcher constantly.
    @lines[line] ||= Line.new line, @identity
    @lines[line].add_monitor(monitor)
  end
  
  def remove_trigger(line_id, rule_id)
    line=@lines[line_id]
    line && line.remove_monitor(rule_id)
  end
  
  def on_readable socket, messages
    messages.each do |m|
      data = nil
      begin
        data = JSON.parse m.copy_out_string
      rescue =>  e
      end
      next unless data
      data['ports'].each do |chunk|
        k = chunk['port']
        type = chunk['type'].downcase
        line = (@lines[k] ||= Line.new k, @identity)
        @cloud.handle_portchange(k,type) if line.portchanged?(type) # this lets the accessories know
        line.update(chunk,@client)
      end 
    end
  end

end
