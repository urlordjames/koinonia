local script = [[
ws.send(JSON.stringify({
	"type": "plugin",
	"id": %s,
	"msg": window.location.href
}));
]]

local formatted_script = string.format(script, get_id());

function on_join(uuid)
	send_module(uuid, formatted_script)
end

function on_msg(msg)
	print(msg)
end
