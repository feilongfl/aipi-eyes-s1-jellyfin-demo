wget -O wget.pcm http://192.168.10.109:8096/Audio/84f7cdda408d432361c29d6bda9cbc27/universal?Container=pcm&TranscodingContainer=pcm&AudioCodec=pcm_s16le&audioBitRate=16000&api_key=d1e79804ea3b4215aecfc7d9a2043e23
python test.py

xxd -i wget.pcm wget.h
xxd -i test.pcm test.h

cat wget.h | tr ',' '\n' | tr -d ' ' | tr -s '\n' >wget.tr.h
cat test.h | tr ',' '\n' | tr -d ' ' | tr -s '\n' >test.tr.h

diff wget.tr.h test.tr.h > wget-test.diff
