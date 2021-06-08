local script = [[
plugin_callbacks[%d] = function (msg) {
	console.log(msg);
};
]]

local formatted_script = string.format(script, get_id());

function on_join(uuid)
	send_module(uuid, formatted_script)

	local msg = "hello " .. uuid
	print(msg)

	send_msg(uuid, msg)
end

function on_leave(uuid)
	print("goodbye " .. uuid)
end
