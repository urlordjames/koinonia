// why do I need to do this myself?
self.addEventListener("install", function(event) {
	caches.open("koinonia-cache").then(function(cache) {
		return cache.addAll([
			"/",
			"/style.css"
			// probably not a good idea to cache koinonia.js yet
		]);
	});
});

self.addEventListener("fetch", function(event) {
	event.respondWith(caches.match(event.request).then(function(response) {
		return response || fetch(event.request);
	}));
});
