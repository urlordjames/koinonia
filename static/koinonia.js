const url = new URL(window.location.href);
const ws = new WebSocket("ws://" + url.hostname + ":" + url.port + "/stream");
const join_button = document.getElementById("join_button");
const participant_div = document.getElementById("participant_div");

let participants = {}
let uuid;

ws.onopen = async function() {
	ws.send(JSON.stringify({"type": "uuid"}));
}

function get_participant(peer_uuid) {
	if (!participants[peer_uuid]) {
		const pc = new RTCPeerConnection();
		const rude = peer_uuid < uuid;

		let participant = participants[peer_uuid];

		participant = {
			"pc": pc,
			"rude": rude
		};

		participants[peer_uuid] = participant

		return participant;
	} else return participants[peer_uuid];
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
			const part = get_participant(peer.uuid);
			const pc = part["pc"];

			pc.createDataChannel("test");

			if (part["rude"]) {
				const offer = await pc.createOffer()

				pc.setLocalDescription(offer);

				ws.send(JSON.stringify({
					"type": "offer",
					"uuid": peer.uuid,
					"offer": offer
				}));
			}

			pc.onicecandidate = function(e) {
				if (e.candidate) {
					ws.send(JSON.stringify({
						"type": "ice",
						"uuid": peer.uuid,
						"candidate": e.candidate
					}));
				}
			}

			pc.ondatachannel = function(c) {
				console.log(c);
			}
		}
	} else if (msg["type"] == "offer") {
		const pc = get_participant(msg["uuid"])["pc"];
		const desc = new RTCSessionDescription(msg["offer"]);
		pc.setRemoteDescription(desc);

		const ans = await pc.createAnswer();
		pc.setLocalDescription(ans);

		ws.send(JSON.stringify({
			"type": "answer",
			"uuid": msg["uuid"],
			"answer": ans
		}));
	} else if (msg["type"] == "answer") {
		const pc = get_participant(msg["uuid"])["pc"];
		const desc = new RTCSessionDescription(msg["answer"]);
		pc.setRemoteDescription(desc);
	} else if (msg["type"] == "ice") {
		const pc = get_participant(msg["uuid"])["pc"];
		pc.addIceCandidate(msg["candidate"])
	}
}
