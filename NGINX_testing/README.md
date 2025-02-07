
Start nginx with custom config file
```
sudo nginx -c "$PWD"/my_nginx.conf 
```

Stop (which??) nginx server -- it works though
```
sudo nginx -s quit
```

Connect to the nginx server
```
telnet localhost 8080
```

> Trying 127.0.0.1...
> Connected to localhost.
> Escape character is '^]'.

Send a request to the nginx server
```
GET /test.txt
```
