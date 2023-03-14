# socketstarter

This program is used as a method to automatically start infrequently used
services on demand.  This can be helpful for services that take too much CPU
or memory when idle.  On startup, this service will bind to the
application's port.  Upon receiving a connection, it will release the port,
start the service, and proxy that first connection to the service. 
Subsequent connections should then go directly to the service.  After the
first connection and a prescribed waiting period is done, the service will
automatically be stopped.  Ideally used for applications which do not
conform with
[systemd socket activation](https://www.freedesktop.org/software/systemd/man/systemd-socket-activate.html).

## Configuration

All configuration is from environment variables.  All are required.
* `PORT` - Port to bind to
* `START_COMMAND` - Command to start application
* `STOP_COMMAND` - Command to stop application
* `SHUTDOWN` - Time, in seconds, to shutdown application automatically after starting
* `TIMEOUT` - Time, in seconds, to wait for application to start

## Systemd Example

`socketstarter.service`
```
[Unit]
Description=Socketstarter
After=network.target

[Service]
User=root
Type=simple
EnvironmentFile=/etc/default/socketstarter
ExecStart=/usr/local/bin/socketstarter
ExecStop=/bin/kill -s STOP $MAINPID
NoNewPrivileges=true
Restart=always
RestartSec=120s

[Install]
WantedBy=multi-user.target
```

`/etc/default/socketstarter`
```
TIMEOUT=60
SHUTDOWN=28800
PORT=8888
START_COMMAND="systemctl start myservice"
STOP_COMMAND="systemctl stop myservice"
```
