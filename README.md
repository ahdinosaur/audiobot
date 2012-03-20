# audiobot

audiobot is designed to fulfill the creators' audio needs

## status

useless and unusable, it's a barfing of code. i need to learn anyways.

simulataneously:

    % ./client
    connecting to tcp://localhost:5555
    request: hi, i'm a message
    response: hi, i'm a message
    request: ^C

    % while IFS= read -r song; do ./server "file:///storage/Music/$song"; done < <(ls /storage/Music/ | shuf)
    version: GStreamer 0.10.35 !
    12-03-16 03:58:24 now playing: file:///storage/Music/Swede Dreams.mp3
    12-03-16 03:59:19 message: hi, i'm a message
    ^C
    
## overiew

audiobot receives requests via the request/response paradigm in zeromq and services the request

### example requests

    "play http://mymusicsite.com/song.mp3"
    "volume 0.55"
    "pause"
    "seek 10:20"

## libraries used

- gstreamer: http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/index.html
- zeromq: http://zguide.zeromq.org/page:all

## license

licensed under the LPGL:

    Copyright 2012 Michael Williams

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU LGPL along with this
    program. If not, see <http://www.gnu.org/licenses/>.
