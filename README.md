# ngx_shellshocked

This project is an NGINX module designed to block the
[Shellshocked](https://www.us-cert.gov/ncas/alerts/TA14-268A) payload
when presented to the server. The module currently checks the host and
referer headers for the payload. To enable the module set
`shellshocked on` in the main conf section of your NGINX
configuration. The following example demonstrates the configuration.

```conf
events {
  worker_connections 1024;
}

http {
  shellshocked on;

  server {
    listen 8888;
    location / {

    }
  }
}
```
