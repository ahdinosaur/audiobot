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
