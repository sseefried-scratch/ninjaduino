a collection of bugs, loosely held together with sockets.

At the top level, we have a single "serial_events" inproc endpoint.
It spits down the line, setting a topic of "line0003" etc for each message.

the next level down, each line creates its own inproc endpoint -
inproc://lineXXXX is probably as good as any. topic will be the
currently active channel.

finally, the monitors and triggers are listening on the appropriate
lineXXXX. topics at this level are to do with the data -
CHANNEL_CHANGE, VALUE, and CLEAR_MONITORS, for instance. Monitors &
triggers listen as appropriate.
