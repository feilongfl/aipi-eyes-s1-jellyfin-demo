
# ffmpeg -y -i d.mp3 -acodec pcm_s16le -f s16le -ac 2 -ar 16000 16k.pcm
# # ffplay -ar 44100 -ac 2 -f s16le 16k.pcm
# xxd -i 16k.pcm 16kpcm.h

curl 'http://192.168.10.109:8096/Audio/84f7cdda408d432361c29d6bda9cbc27/universal?UserId=0a0dd21302ab4484a0d5c4cf2952f805&DeviceId=TW96aWxsYS81LjAgKFgxMTsgTGludXggeDg2XzY0KSBBcHBsZVdlYktpdC81MzcuMzYgKEtIVE1MLCBsaWtlIEdlY2tvKSBDaHJvbWUvMTE2LjAuMC4wIFNhZmFyaS81MzcuMzZ8MTY5NDkwNjk3MDc0Mg11&MaxStreamingBitrate=140000000&Container=wav&TranscodingContainer=wav&TranscodingProtocol=pcm&AudioCodec=pcm_s16le&audioBitRate=16000&api_key=dbd508dea9b249648b170318145d326c&PlaySessionId=1696206413077&StartTimeTicks=0&EnableRedirection=true&EnableRemoteMedia=false' \
                  -H 'Cache-Control: no-cache' \
                  -H 'DNT: 1' \
                  -H 'Pragma: no-cache' \
                  -H 'Range: bytes=0-' \
                  -H 'User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36' -o test.pcm
xxd -i test.pcm test.pcm.h
