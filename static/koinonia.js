const url = new URL(window.location.href);
const ws = new WebSocket("ws://" + url.hostname + ":" + url.port + "/stream");
const permissions_button = document.getElementById("permissions_button");
const join_button = document.getElementById("join_button");
const participant_div = document.getElementById("participant_div");

let participants = {}
let uuid;

permissions_button.addEventListener("click", async function(e) {
	const localStream = await navigator.mediaDevices.getDisplayMedia({
		"video": true,
		"audio": false
	});

	for (participant of Object.values(participants)) {
		const pc = participant["pc"];

		for (track of localStream.getTracks()) {
			pc.addTrack(track, localStream);
		}
	}
});

ws.onopen = async function() {
	ws.send(JSON.stringify({"type": "uuid"}));
}

function get_participant(peer_uuid) {
	if (!participants[peer_uuid]) {
		const pc = new RTCPeerConnection();

		let participant = participants[peer_uuid];

		participant = {
			"pc": pc,
		};

		participants[peer_uuid] = participant

		pc.onnegotiationneeded = async function() {
			const offer = await pc.createOffer()

			pc.setLocalDescription(offer);

			ws.send(JSON.stringify({
				"type": "offer",
				"uuid": peer_uuid,
				"offer": offer
			}));
		}

		pc.onicecandidate = function(e) {
			if (e.candidate) {
				ws.send(JSON.stringify({
					"type": "ice",
					"uuid": peer_uuid,
					"candidate": e.candidate
				}));
			}
		}

		if (peer_uuid < uuid) {
			pc.onnegotiationneeded();
		}

		pc.ontrack = function(e) {
			const video = document.createElement("video");
			const remoteStream = new MediaStream();

			for (track of e.streams[0].getTracks()) {
				remoteStream.addTrack(track);
			}

			video.srcObject = remoteStream;
			participant_div.appendChild(video);
			video.play();
		}

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
			// refresh local participant list
			get_participant(peer.uuid);
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
	} else if (msg["type"] == "negotiate") {
		const pc = get_participant(msg["uuid"])["pc"];
		pc.onnegotiationneeded();
	} else if (msg["type"] == "debug") {
		console.log(msg["message"]);
	} else if (msg["type"] == "error") {
		alert(msg["message"]);
	} else if (msg["type"] == "leave") {
		const part = participants[msg["uuid"]]
		if (part) {
			part["pc"].close()
			delete participants[msg["uuid"]];
		}
	}
}
