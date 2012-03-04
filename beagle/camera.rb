require 'ninja_blocks'

class Camera
  def initialize(data)
    @block_id = data['block']
  end

  def worker?
    true
  end

  def client?
    true
  end

  def set_client(client)
    @client = client
  end
  
  def do_action_take_photo(datachunk,rule_id)
    # fake it out for the moment: sub in real camera once it stops
    # segfaulting
    if !@client
      warn "asked to take picture, but we have no client, so no response could be made"
      return "ok"
    end

    puts "taking a picture!"
    system("uvccapture")
    file_contents = File.open("snap.jpg").read
    req = NinjaBlocks::LookupRequest.new do
      service_name datachunk['service']
      data         ({ :environment => datachunk['data'], :photo => file_contents })
      message_type "do"
      entity_type "action"
      name datachunk['callback']
      rule_id rule_id
    end
    @client.process_request req
    puts "response sent!"

    "ok"
    # so really, we want to send a message to the other service
    # let's try the dumbest thing that could possibly work.
    
  end


  
end



