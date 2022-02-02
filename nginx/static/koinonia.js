const url = new URL(window.location.href);

const room_parameter = url.searchParams.get("room");
if (room_parameter !== null) {
	sessionStorage.setItem("room_id", room_parameter);
	location = "/";
}

const ws_prefix = (url.protocol == "https:") ? "wss://" : "ws://";

const n_room = sessionStorage.getItem("room_id") || "0";
const room_id_input = document.getElementById("room_id");
room_id_input.value = n_room;

const ws = new WebSocket(ws_prefix + url.hostname + ":" + url.port + "/stream?id=" + n_room);

const screenshare_button = document.getElementById("screenshare_button");
const camera_button = document.getElementById("camera_button");
const mic_button = document.getElementById("mic_button");
// on firefox even though the button is set to disabled in HTML, it becomes enabled again when you reload
screenshare_button.disabled = true;
camera_button.disabled = true;
mic_button.disabled = true;

const room_button = document.getElementById("change_room_button");
room_button.addEventListener("click", function (e) {
	sessionStorage.setItem("room_id", room_id_input.value);
	location.reload();
});

const participant_div = document.getElementById("participant_div");

if ("serviceWorker" in navigator) {
	navigator.serviceWorker.register(new URL("sw.js", import.meta.url));
}

let participants = {}
let uuid;
let local_streams = new Set();

function add_tracks(tracks, flip) {
	for (const track of tracks) {
		local_streams.add(track);

		track.onended = function () {
			local_streams.remove(track);
		};

		for (const participant of Object.values(participants)) {
			const pc = participant["pc"];
			increase_quality(pc.addTrack(track));
		}

		// don't play audio
		if (track.kind === "video") {
			play_track(track, flip);
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
		add_tracks(localStream.getTracks(), false);
	});
});

camera_button.addEventListener("click", function(e) {
	navigator.mediaDevices.getUserMedia({
		"video": {"facingMode": "user"}
	}).then(function(localStream) {
		add_tracks(localStream.getTracks(), true);
	});
});

mic_button.addEventListener("click", function(e) {
	navigator.mediaDevices.getUserMedia({
		"audio": true
	}).then(function(localStream) {
		add_tracks(localStream.getTracks(), true);
	});
});

ws.onopen = async function() {
	ws.send(JSON.stringify({"type": "uuid"}));
}

ws.onclose = function() {
	screenshare_button.disabled = true;
	camera_button.disabled = true;
	mic_button.disabled = true;
}

let rtc_config = {
	"iceServers": [
		{
			"urls": ["stun:" + url.hostname]
		},
		{
			"urls": ["turn:" + url.hostname],
			"username": "koinonia",
			"credential": "koinonia"
		}
	]
}

// for handling autoplay
let added_button = false;
let blocked_tracks = [];

function play_track(track, flip) {
	const track_element = document.createElement(track.kind);

	const remoteStream = new MediaStream();
	remoteStream.addTrack(track);

	track_element.srcObject = remoteStream;
	track_element.style.width = "100%";

	// people don't generally like seeing themselves not mirrored
	if (flip) {
		track_element.style.transform = "scale(-1, 1)";
	}

	participant_div.appendChild(track_element);

	track_element.play().catch(function(error) {
		if (error.name === "NotAllowedError") {
			blocked_tracks.push(track_element);

			if (!added_button) {
				const autoplay_div = document.createElement("div");

				const info_text = document.createTextNode("your browser is preventing one or more streams from playing");
				autoplay_div.appendChild(info_text);

				const play_button = document.createElement("button");
				play_button.innerHTML = "click me to fix";
				play_button.onclick = async function() {
					for (const track of blocked_tracks) {
						track.play();
					}
					blocked_tracks = null;
					autoplay_div.remove();
				};
				autoplay_div.appendChild(play_button);

				document.body.insertBefore(autoplay_div, document.body.firstChild);
				added_button = true;
			}
		} else {
			alert("track playback failed, see developer console for more info");
			console.log(error);
		}
	});

	track.onended = function() {
		track_element.remove();
	}
}

function get_participant(peer_uuid) {
	if (!participants[peer_uuid]) {
		const pc = new RTCPeerConnection(rtc_config);

		let participant = {
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
			play_track(e.track, false);
		}

		for (const track of local_streams) {
			increase_quality(pc.addTrack(track));
		}

		return participant;
	} else return participants[peer_uuid];
}

// it should work I think...
function increase_quality(rtp_sender) {
	setTimeout(function() {
		let params = rtp_sender.getParameters();
		let encoding = params.encodings[0];
		encoding.maxBitrate = 8 * 1000 * 1000;
		encoding.maxFramerate = 60;
	}, 3000);
}

let plugin_callbacks = {};

ws.onmessage = async function(e) {
	const msg = JSON.parse(e.data);
	if (msg["type"] == "uuid") {
		uuid = msg.uuid;
		screenshare_button.disabled = false;
		camera_button.disabled = false;
		mic_button.disabled = false;
	} else if (msg["type"] == "sync") {
		for (const peer of msg.peers) {
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
