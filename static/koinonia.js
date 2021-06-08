const url = new URL(window.location.href);
const ws_prefix = (url.protocol == "https:") ? "wss://" : "ws://";
const ws = new WebSocket(ws_prefix + url.hostname + ":" + url.port + "/stream");

const screenshare_button = document.getElementById("screenshare_button");
const camera_button = document.getElementById("camera_button");
// on firefox even though the button is set to disabled in HTML, it becomes enabled again when you reload
screenshare_button.disabled = true;
camera_button.disabled = true;

const participant_div = document.getElementById("participant_div");

let participants = {}
let uuid;
let local_streams = new Set();

function add_tracks(tracks) {
	for (track of tracks) {
		local_streams.add(track);

		track.onended = function () {
			local_streams.remove(track);
		};

		for (participant of Object.values(participants)) {
			const pc = participant["pc"];
			pc.addTrack(track);
		}
	}
}

screenshare_button.addEventListener("click", function(e) {
	navigator.mediaDevices.getDisplayMedia({
		"video": true,
		// audio does not work on all browsers that support getDisplayMedia
		// see https://caniuse.com/mdn-api_mediadevices_getdisplaymedia_audio-capture-support
		"audio": true
	}).then(function (localStream) {
		add_tracks(localStream.getTracks());
	});
});

camera_button.addEventListener("click", function(e) {
	navigator.mediaDevices.getUserMedia({
		"video": {"facingMode": "user"}
	}).then(function(localStream) {
		add_tracks(localStream.getTracks());
	});
});

ws.onopen = async function() {
	ws.send(JSON.stringify({"type": "uuid"}));
}

ws.onclose = function() {
	screenshare_button.disabled = true;
	camera_button.disabled = true;
}

let rtc_config = {}

fetch("/rtc_config.txt").then(function (resp) {
	if (!resp.ok) {
		console.error("failed to fetch rtc config");
		return;
	}

	resp.json().then(function (json) {
		rtc_config = json;
	});
});

function get_participant(peer_uuid) {
	if (!participants[peer_uuid]) {
		const pc = new RTCPeerConnection(rtc_config);

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
			const track_element = document.createElement(e.track.kind);

			const remoteStream = new MediaStream();
			remoteStream.addTrack(e.track);

			track_element.srcObject = remoteStream;
			participant_div.appendChild(track_element);
			// TODO: deal with autoplay disabled issues
			track_element.play();

			e.track.onended = function() {
				track_element.remove();
			}
		}

		for (track of local_streams) {
			pc.addTrack(track);
		}

		return participant;
	} else return participants[peer_uuid];
}

let plugin_callbacks = {};

ws.onmessage = async function(e) {
	const msg = JSON.parse(e.data);
	if (msg["type"] == "uuid") {
		uuid = msg.uuid;
		screenshare_button.disabled = false;
		camera_button.disabled = false;
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
		console.log(msg["msg"]);
	} else if (msg["type"] == "error") {
		alert(msg["msg"]);
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
	} else if (msg["type"] == "plugin") {
		plugin_callbacks[msg["id"]](msg["msg"]);
	}
}
