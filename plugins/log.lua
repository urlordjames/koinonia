local script = [[
let mydiv = document.createElement("div");
document.getElementById("plugin_div").appendChild(mydiv);

let myinput = document.createElement("input");
myinput.value = "some text";
mydiv.appendChild(myinput);

let mybutton = document.createElement("button");
mybutton.innerHTML = "log";
mydiv.appendChild(mybutton);

mybutton.addEventListener("click", async function() {
	ws.send(JSON.stringify({
		"type": "plugin",
		"id": %s,
		"msg": myinput.value
	}));
});
]]

local formatted_script = string.format(script, get_id());

function on_join(uuid)
	send_module(uuid, formatted_script)
end

function on_msg(msg)
	print(msg)
end
