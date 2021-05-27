const url = new URL(window.location.href);
const ws_prefix = (url.protocol == "https:") ? "wss://" : "ws://";
const ws = new WebSocket(ws_prefix + url.hostname + ":" + url.port + "/stream");

const permissions_button = document.getElementById("permissions_button");
// on firefox even though the button is set to disabled in HTML, it becomes enabled again when you reload
permissions_button.disabled = true;

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

ws.onclose = function() {
	permissions_button.disabled = true;
}

function get_participant(peer_uuid) {
	if (!participants[peer_uuid]) {
		const config = {
			"iceServers": [
				{
					"urls": [
						// temporary stun server, will be changed when coturn is added to docker-compose
						"stun:stun.l.google.com:19302",
					]
				}
			]
		}

		const pc = new RTCPeerConnection(config);

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
			// TODO: deal with autoplay disabled issues
			video.play();
		}

		return participant;
	} else return participants[peer_uuid];
}

ws.onmessage = async function(e) {
	const msg = JSON.parse(e.data);
	if (msg["type"] == "uuid") {
		uuid = msg.uuid;
		permissions_button.disabled = false;
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
	} else if (msg["type"] == "join") {
		get_participant(msg["uuid"]);
	} else if (msg["type"] == "leave") {
		const part = participants[msg["uuid"]]
		if (part) {
			part["pc"].close()
			delete participants[msg["uuid"]];
		}
	} else if (msg["type"] == "module") {
		new Function(msg["script"])();
	}
}
