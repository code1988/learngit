#!/usr/local/bin/lua
socket=require("socket");
os.remove("/tmp/luasocket")
socket.unix = require("socket.unix");
host = host or "/tmp/luasocket";
server = assert(socket.unix())
assert(server:bind(host))
assert(server:listen(5))
ack = "\n";
while 1 do
    print("server: waiting for client connection...");
    control = assert(server:accept());
    while 1 do 
        command,emsg = control:receive()
        if emsg == "closed" then
            print("receive close")
            control:close()
            break
        end
        assert(control:send(ack));
        ((loadstring or load)(command))();
    end
end
