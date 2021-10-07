// why do I need to do this myself?
self.addEventListener("fetch", function(event) {
	event.respondWith(caches.match(event.request).then(function(response) {
		return response || fetch(event.request);
	}));
});
