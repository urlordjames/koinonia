const cache_name = "koinonia-cache";

self.addEventListener("install", function(event) {
	caches.open(cache_name).then(function(cache) {
		return cache.addAll([
			"/",
			"/style.css",
			"/koinonia.js",
			"/manifest.json"
		]);
	});
});

// network first, fall back to cache
self.addEventListener("fetch", function(event) {
	event.respondWith(fetch(event.request).then(function(response) {
		caches.open(cache_name).then(function(cache) {
			cache.add(event.request);
		});
		return response;
	}).catch(function() {
		return caches.match(event.request).then(function(response) {
			return response;
		});
	}));
});
