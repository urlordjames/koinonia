function on_join(uuid)
	local msg = "hello " .. uuid
	print(msg)
	send_msg(uuid, msg)
end
