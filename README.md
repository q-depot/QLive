#TODO

Fix this
QLive::parseDeviceParam( osc::Message message )
line #439

LiveGui: 

    - volume and param slider don't get set properly
    - implement even handlers
    - update gui on msg received
    - params values must be updated with the value ref, the current solution goes in conflicts because osc runs in a thread. the gui often override ableton(try to move the knobs)
    
    
QLive isAlive(): what do I need it for? is it safe to ALWAYS set the local value and then send out the osc msg to Live or is better to only send hte osc msg and wait for the response?
