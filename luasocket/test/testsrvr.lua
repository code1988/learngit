#!/usr/local/bin/lua
socket = require("socket");
host = host or "localhost";
port = port or "8383";
server = assert(socket.bind(host, port));
ack = "\n";
while 1 do
    print("server: waiting for client connection...");
    control = assert(server:accept());
    while 1 do
        print("receive...")
        command, emsg = control:receive();
        if emsg == "closed" then
            print("receive close")
            control:close()
            break
        end
        assert(command, emsg)
        assert(control:send(ack));
        print(command);
		((loadstring or load)(command))();
    end
end
