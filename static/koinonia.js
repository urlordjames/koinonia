const url = new URL(window.location.href);
const ws = new WebSocket("ws://" + url.hostname + ":" + url.port + "/stream");
const join_button = document.getElementById("join_button");
const participant_div = document.getElementById("participant_div");

let participants = {}
let uuid;

ws.onopen = async function() {
	ws.send(JSON.stringify({"type": "uuid"}));
}

ws.onmessage = async function(e) {
	const msg = JSON.parse(e.data);
	if (msg["type"] == "uuid") {
		uuid = msg.uuid;
		join_button.disabled = false;
		join_button.addEventListener("click", function(e) {
			ws.send(JSON.stringify({"type": "sync"}));
			join_button.remove();
			delete join_button;
		});
	} else if (msg["type"] == "sync") {
		for (peer of msg.peers) {
			if (participants[peer.uuid] == null) {
				const pc = new RTCPeerConnection();
				const rude = peer.uuid < uuid;

				participants[peer.uuid] = {
					"pc": pc,
					"rude": rude 
				};

				if (rude) {
					const offer = await pc.createOffer()

					pc.setLocalDescription(offer);

					ws.send(JSON.stringify({
						"type": "offer",
						"uuid": peer.uuid,
						"offer": offer
					}));
				}
			}
		}
	} else if (msg["type"] == "offer") {
		const pc = participants[msg["uuid"]]["pc"];
		const desc = new RTCSessionDescription(msg["offer"]);
		pc.setRemoteDescription(desc);

		const ans = pc.createAnswer();
		console.log(ans);
	}
}
