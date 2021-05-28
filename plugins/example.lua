function on_join(uuid)
	local msg = "hello " .. uuid
	print(msg)
	send_module(uuid, "console.log(\"" .. msg .. "\")")
	send_msg(uuid, msg)
end
