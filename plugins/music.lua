local script = [[
let tag = document.createElement("script");
tag.src = "https://www.youtube.com/iframe_api";
document.head.appendChild(tag);

let player_div = document.createElement("div");
player_div.style = "display: none";
document.body.appendChild(player_div);

onYouTubeIframeAPIReady = function() {
	player = new YT.Player(player_div, {
		"videoId": "2ZIpFytCSVc",
		"events": {
			"onReady": onPlayerReady
		}
	});
};

onPlayerReady = function(e) {
	e.target.playVideo();
}
]]

function on_join(uuid)
	send_module(uuid, script)
end
