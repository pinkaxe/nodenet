#!/bin/bash

# to test error codes, all commands invalid in some way

HOST=127.0.0.1
PORT=9999

while true; do
	./cli $HOST	8999 PUT "images/lips.png";
	./cli $HOST $PORT PUT1 "images/lips.png";
	./cli $HOST $PORT PUT "/usr/share/doc/espeak/images/lips.p";
	./cli $HOST $PORT PUT "/usr/share/doc/espeak/images/lips.png";
done;

