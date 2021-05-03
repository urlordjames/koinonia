// this may not even be useful anymore because the server has changed so much...

const url = new URL(window.location.href);
const ws = new WebSocket("ws://" + url.hostname + ":" + url.port + "/stream");
const div = document.getElementById("wslog");
const text = document.getElementById("sockettext");
const button = document.getElementById("sendbutton");

let pc = new RTCPeerConnection({
	"iceServers": [{
		urls: ["stun:stun.l.google.com:19302"]
	}]
});
let dc = pc.createDataChannel("test");
let offer = pc.createOffer();

ws.onopen = async function() {
	sendMsg(JSON.stringify({"type": "join", "sdp": await offer}));
	sendMsg(JSON.stringify({"type": "sync"}));
	button.disabled = false;
	pc.setLocalDescription(await offer);
}

ws.onclose = function() {
	button.disabled = true;
	
	let newdiv = document.createElement("div");
	newdiv.setAttribute("error", "true");
	newdiv.innerHTML = "connection closed";
	div.appendChild(newdiv);
}

ws.onmessage = async function(msg) {
	let newdiv = document.createElement("div");
	newdiv.setAttribute("side", "server");
	newdiv.innerHTML = msg.data;
	div.appendChild(newdiv);
	const data = JSON.parse(msg.data);
	if (data["type"] == "sync") {
		if (data["peers"][0]) {
			let peer = data["peers"][0];
			let desc = new RTCSessionDescription(JSON.parse(peer["sdp"]));
			pc.setRemoteDescription(desc);
			let ans = pc.createAnswer();
			pc.setLocalDescription(await ans);
			// uhh, this is an oversight, in the final version there can't be a global peer connection variable
			// it will probably need to be created in response to a sync message
			peer_uuid = peer["uuid"];
			text.value = JSON.stringify({"type": "answer", "uuid": peer_uuid, "message": await ans});
		}
	} else if (data["type"] == "answer") {
		let msg = data["message"];
		let desc = new RTCSessionDescription(msg);
		pc.setRemoteDescription(desc);
	} else if (data["type"] == "ice") {
		const candidate = data["candidate"];
		pc.addIceCandidate(candidate);
	}
}

pc.ondatachannel = function(c) {
	console.log(c);
}

pc.onicecandidate = function(e) {
	if (e.candidate == null) return;
	sendMsg(JSON.stringify({"type": "ice", "uuid": peer_uuid, "candidate": JSON.stringify(e.candidate)}));
}

button.addEventListener("click", onClick);

function onClick() {
	sendMsg(text.value);
}

function sendMsg(msg) {
	let newdiv = document.createElement("div");
	newdiv.setAttribute("side", "client");
	newdiv.innerHTML = msg;
	div.appendChild(newdiv);
	
	ws.send(msg);
}
