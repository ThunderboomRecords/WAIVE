#!/bin/bash

magick ../assets/WAIVE_Midi.png \
\( +clone  -alpha extract \
    -draw 'fill black polygon 0,0 0,5 5,0 fill white circle 5,5 5,0' \
    \( +clone -flip \) -compose Multiply -composite \
    \( +clone -flop \) -compose Multiply -composite \
\) -alpha off -compose CopyOpacity -composite \
/tmp/rounded.png

magick /tmp/rounded.png \
\( -clone 0 -background gray -shadow 20x8+4+8 \) \
\( -clone 0 -background gray -shadow 19x8+4+20 \) \
-reverse -background none -layers merge +repage \
../assets/WAIVE_Midi_preview.png

magick ../assets/WAIVE_Sampler.png \
\( +clone  -alpha extract \
    -draw 'fill black polygon 0,0 0,5 5,0 fill white circle 5,5 5,0' \
    \( +clone -flip \) -compose Multiply -composite \
    \( +clone -flop \) -compose Multiply -composite \
\) -alpha off -compose CopyOpacity -composite \
/tmp/rounded.png

magick /tmp/rounded.png \
\( -clone 0 -background gray -shadow 20x8+4+8 \) \
\( -clone 0 -background gray -shadow 19x8+4+20 \) \
-reverse -background none -layers merge +repage \
../assets/WAIVE_Sampler_preview.png