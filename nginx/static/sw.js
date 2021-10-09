self.addEventListener("install", function(event) {
	caches.open("koinonia-cache").then(function(cache) {
		return cache.addAll([
			"/",
			"/style.css"
			// probably not a good idea to cache koinonia.js yet
		]);
	});
});

// network first, fall back to cache
self.addEventListener("fetch", function(event) {
	event.respondWith(fetch(event.request).then(function(response) {
		return response;
	}).catch(function() {
		return caches.match(event.request).then(function(response) {
			return response;
		});
	}));
});
