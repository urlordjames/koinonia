local script = [[
plugin_callbacks[%d] = function (msg) {
	console.log(msg);
};
]]

local formatted_script = string.format(script, get_id());

function on_join(uuid)
	send_module(uuid, formatted_script)
	send_msg(uuid, "hello")
end
