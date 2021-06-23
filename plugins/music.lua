local init_script = [[
let tag = document.createElement("script");
tag.src = "https://www.youtube.com/iframe_api";
document.head.appendChild(tag);

onYouTubeIframeAPIReady = function() {
	ws.send(JSON.stringify({
		"type": "plugin",
		"id": %d,
		"msg": "ready"
	}));
};
]]

local formatted_init_script = string.format(init_script, get_id())

local script = [[
let player_div = document.createElement("div");
player_div.style = "display: none";
document.body.appendChild(player_div);

player = new YT.Player(player_div, {
	"videoId": "%s",
	"events": {
		"onReady": function(e) {
			e.target.playVideo();
		}
	}
});
]]

function on_join(uuid)
	send_module(uuid, formatted_init_script)
end

local function send_media(uuid, media_id)
	send_module(uuid, string.format(script, media_id))
end

local ready_uuids = {}

function on_msg(uuid, msg)
	if msg == "ready" then
		table.insert(ready_uuids, uuid)
	else
		for i = 1,#ready_uuids do
			send_media(ready_uuids[i], msg)
		end
	end
end

function on_leave(uuid)
	for i = 1,#ready_uuids do
		if ready_uuids[i] == uuid then
			table.remove(ready_uuids, i)
			return
		end
	end
end
