const url = new URL(window.location.href);
const ws = new WebSocket("ws://" + url.hostname + ":" + url.port + "/stream");
const div = document.getElementById("wslog");
const text = document.getElementById("sockettext");
const button = document.getElementById("sendbutton");

let pc = new RTCPeerConnection();
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
			await pc.setRemoteDescription(desc);
			let ans = pc.createAnswer();
			pc.setLocalDescription(await ans);
			console.log(JSON.stringify(await ans));
			text.value = JSON.stringify({"type": "answer", "uuid": peer["uuid"], "message": await ans});
		}
	}
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
