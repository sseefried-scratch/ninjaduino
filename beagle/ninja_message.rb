require 'rubygems'
require 'msgpack'

class NinjaMessage
  def self.create_from(worker, message)
    if message.request?
      LookupRequest.from_message(worker, message)
    elsif message.success_reply?
      LookupReplySuccess.from_message(message)
    elsif message.failure_reply?
      LookupReplyFailure.from_message(message)
    end
  end

  def self.decode_payload(payload_strings)
    if payload_strings
      payload_strings.map { |string| MessagePack.unpack(string) } 
    else
      # huh. really?
      Array.new(1) { Hash.new }
    end
  end

  def self.encode_payload(hsh)
    hsh.to_msgpack
  end

  def self.create_accessors(mod, fields)
    fields.each do |field_name|
      code = <<-code
      def #{ field_name } (value = nil)
        if value
          @#{field_name} = value
        else
          @#{field_name}
        end
      end

      def #{ field_name }=(value)
        @#{field_name} = value
      end
      code

      mod.class_eval code
    end
  end

  # the Majordomo messages expect all payloads to be wrapped in
  # arrays; each element of the array will be sent as a separate
  # frame
  def encode(string)
    [string]
  end
end # NinjaMessage


class LookupRequest < NinjaMessage
  create_accessors(self, %w(worker auth sequence_id service_name name message_type entity_type rule_id data))

  def self.from_message(worker, message)
    payload = decode_payload(message.payload)[0] # only care about
    # first frame
    puts payload.inspect
    message_type = payload['action']
    # action can be delete or add
    # only support add for now
    
    new do
      worker worker
      sequence_id message.sequence_id
      message_type payload['message_type']
      entity_type payload['entity_type'] # trigger or action
      data payload['data']
      name payload['name']
      rule_id payload['rule_id']
      auth payload['auth']
    end
  end

  def initialize(&blk)
    instance_eval(&blk) if block_given?
  end

  def encode
    string = self.class.encode_payload({
                                         'message_type' => message_type,
                                         'entity_type' => entity_type,
                                         'data'=> data,
                                         'rule_id' => rule_id,
                                         'name' => name,
                                         'auth' => auth
                                       })

    super(string)
  end

  def execute

  end
  
end # LookupRequest


class LookupReplySuccess < NinjaMessage
  create_accessors(self, %w(sequence_id answer))

  def self.from_request(request)
    new { sequence_id request.sequence_id }
  end

  def self.from_message(message)
    payload = decode_payload(message.payload)[0] # only care about first frame
    new do
      sequence_id message.sequence_id
      answer payload['answer']
    end
  end

  def initialize(&blk)
    instance_eval(&blk) if block_given?
  end

  def encode
    string = self.class.encode_payload({
      'answer' => answer
    })

    super(string)
  end
end # LookupReplySuccess


class LookupReplyFailure < LookupReplySuccess
end

