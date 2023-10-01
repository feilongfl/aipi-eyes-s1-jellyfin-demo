
ffmpeg -y -i d.mp3 -acodec pcm_s16le -f s16le -ac 2 -ar 16000 16k.pcm
# ffplay -ar 44100 -ac 2 -f s16le 16k.pcm
xxd -i 16k.pcm 16kpcm.h
