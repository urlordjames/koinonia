const url = new URL(window.location.href);
const ws = new WebSocket("ws://" + url.hostname + ":" + url.port + "/stream");
const join_button = document.getElementById("join_button");
const participant_div = document.getElementById("participant_div");

ws.onopen = async function() {
	join_button.disabled = false;
	join_button.addEventListener("click", function(e) {
		console.log("test");
	});
}
