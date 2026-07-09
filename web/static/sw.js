const CACHE_NAME = 'droneops-v0.1.1';
const ASSETS_TO_CACHE = [
  '/static/index.html',
  '/static/app.js',
  '/static/style.css',
  '/static/manifest.json'
];

self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => {
      return cache.addAll(ASSETS_TO_CACHE);
    })
  );
  self.skipWaiting();
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((keyList) => {
      return Promise.all(keyList.map((key) => {
        if (key !== CACHE_NAME) {
          return caches.delete(key);
        }
      }));
    })
  );
  self.clients.claim();
});

self.addEventListener('fetch', (event) => {
  event.respondWith(
    caches.match(event.request).then((response) => {
      return response || fetch(event.request);
    }).catch(() => {
      // Falha de rede e não está no cache.
      // Se for navegação para a raiz ou /static/index.html, retorna o index (offline).
      if (event.request.mode === 'navigate') {
        return caches.match('/static/index.html');
      }
    })
  );
});
