# audiobot

audiobot is designed to fulfill the creators' audio needs

## status

useless and unusable

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
