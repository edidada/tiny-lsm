// giscus.js
window.addEventListener('DOMContentLoaded', function () {
  const giscus = document.createElement('script');
  giscus.src = 'https://giscus.app/client.js';
  giscus.setAttribute('data-repo', 'ToniXWD/toni-lsm');
  giscus.setAttribute('data-repo-id', 'R_kgDONvdi9Q');
  giscus.setAttribute('data-category', 'Announcements');
  giscus.setAttribute('data-category-id', 'DIC_kwDONvdi9c4CqDu8');
  giscus.setAttribute('data-mapping', 'pathname');
  giscus.setAttribute('data-theme', 'preferred_color_scheme');
  giscus.setAttribute('crossorigin', 'anonymous');
  giscus.async = true;
  document.querySelector('main')?.appendChild(giscus);
});
