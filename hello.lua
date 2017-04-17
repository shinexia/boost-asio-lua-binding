local function main()
    local client = require("net.tcp.client")
    local tcp = client.new()

    tcp:setHost("127.0.0.1")
    tcp:setPort(2000)

    tcp:onConnected(function ( endpoint )
        print("conneted to:" .. endpoint)
        tcp:send('{"userName":"shine", "password":"123456"}')
    end)

--    tcp:onClosed(function ()
--        print("tcp connection closed.")
--    end)

--    tcp:onError(function ( e )
--        print("tcp error:" .. e)
--    end)

--    tcp:onMessage(function( msg )
--        print("message:" .. msg)
--    end)

    tcp:connect()

    for i = 1, 3 do
        print("Enter:")
		local x = io.read()      -- produce new value
        tcp:send(x)             -- send to server
    end

	tcp:close()
end

main()