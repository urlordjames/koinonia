const url = new URL(window.location.href);
const ws = new WebSocket("ws://" + url.hostname + ":" + url.port + "/stream");
const div = document.getElementById("wslog");
const text = document.getElementById("sockettext");
const button = document.getElementById("sendbutton");

let pc = new RTCPeerConnection();
let offer = pc.createOffer();

ws.onopen = async function() {
	text.value = JSON.stringify({"type": "join", "sdp": await offer});
	button.disabled = false;
}

ws.onclose = function() {
	button.disabled = true;
	
	let newdiv = document.createElement("div");
	newdiv.setAttribute("error", "true");
	newdiv.innerHTML = "connection closed";
	div.appendChild(newdiv);
}

ws.onmessage = function(msg) {
	let newdiv = document.createElement("div");
	newdiv.setAttribute("side", "server");
	newdiv.innerHTML = msg.data
	div.appendChild(newdiv);
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
