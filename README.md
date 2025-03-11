# WAIVE-Plugins

<p align="center">
    <img src="assets/logo.png">
</p>
<p align="center">
    AI x Archive music tools
</p>

<p align="center">
    <img 
        src="assets/WAIVE_Sequencer.png" 
        width="500"
        alt="WAIVE-Sequencer screenshot"
    >
</p>

<p align="center">
    <img 
        src="assets/WAIVE_Sampler.png" 
        width="500"
        alt="WAIVE-Sampler screenshot"
    >
</p>

A plugin suite that combines music, sound and MIDI generation with European cultural archives. 
Aims to be an offline, modular version of [WAIVE-Studio](https://www.waive.studio/) that can be integrated into your DAW.
Built with [DISTRHO Plugin Framework](https://github.com/DISTRHO/DPF) and [ONNX Runtime](https://github.com/microsoft/onnxruntime).

- **WAIVE-Sequencer**: a rhythmic pattern generator
- **WAIVE-Sampler**: a sample player, sample library and sample generator all in one

Developed by [Arran Lyon](https://arranlyon.com) for [Thunderboom Records](https://www.thunderboomrecords.com). Contributions and pull-requests welcome, especially regarding stability and performance improvements.

## Download and Install
WAIVE-Sampler and WAIVE-Sequencer are available in VST, VST3 (macOS + Linux only), CLAP and Audio Unit (macOS only) plugin formats. 

1. Download the installer from the [**Releases**](https://github.com/ThunderboomRecords/WAIVE/releases) page for your platform. You can find the download links under the **Assets** heading. For newer Apple computers with M series processors, use `macOS_arm64`, whereas older machines with Intel chips should use `macOS_x64`. 
2. Install:
    - MacOS: open the `.dmg` file, open the `.pkg` and follow the instructions.
    - Windows: run the installer `.exe` and follow the instructions.
    - Linux: extract the zip and move the plugins to you prefered plugin folder (e.g. `/usr/lib/vst3`)
3. In your DAW, rescan plugins if it does not do so automatically.

### Ableton Live usage notes
- On Windows: Ableton may not search in the installtion location of the plugins. Go to Options > Preferences > Plugins > Plug-In Sources, make sure 'Use VST Plug-In Custom Folder' is ON, and click Browse and go to `C:\Program Files\Common Files\VST2` and click 'Select Folder', then 'Rescan'.
- To use WAIVE-Sequencer, add this plugin to an empty MIDI track. Then, on a separate MIDI track add your instrument (e.g. WAIVE-Sampler) then set MIDI From to the name of the first track and make sure to set it to use Post-FX.

## Build from source (for developers)
If you want to build WAIVE-Plugins for your platform from source, follow the separate instructions [here](BUILD_INSTRUCTIONS.md).

## Troubleshooting
### WAIVE-Sampler not loading sources list
- Make sure you are connected to the internet
- Click "View Folder" button to open up the location the database is saved in you file browser. Delete the file WAIVE.db, close and remove the plugin from the track, and re-add it and re-open.
  - On macOS, this is located at `/Users/[your username]/Library/Application Support/WAIVE`
  - On Windows, this is located at `C:\Users\[your username]\App Data\Local\WAIVE`
 
## About
WAIVE-Plugins are built on top of several bespoke music and audio analysis models in order to create a unique and custom musical toolbox. These models will continue to be fine-tuned and updated on new data. 

**WAIVE-Sequencer** utilises a flexible approach of modelling rhythmic data first outlined by Jon Gillick and collaborators. This works by encoding a percussion loop into it's *Score* and *Groove* components, then a model (a Variational AutoEncoder, *VAE*) is trained to combine them into a final output sequence. By adjusting the groove information, the same pattern (score) can be played in a different style. For the plugin, we also trained 2 smaller models (VAEs) that can encode the training data and can generate new Grooves and Scores in a variety of styles. 

We use a combination of MIDI data and raw audio to collect rhythm information to train the models. In the case of raw audio, first the percussion instruments are isolated using [demucs](https://github.com/facebookresearch/demucs), then a method (Non-Negative Matrix Factorisation, demoed [here](https://www.audiolabs-erlangen.de/resources/MIR/2016-IEEE-TASLP-DrumSeparation/AmenBreak)) to further separate the individual percussion instruments is used to extract the score and groove information. 

Rhythmic datasets include:
- [Groove MIDI Dataset](https://magenta.tensorflow.org/datasets/groove) from Magenta
- [Folkloras](http://etnomuzikologai.lmta.lt/), Baltic folk music archive.
- [Nederlandse Liederenbank](https://www.liederenbank.nl/), Dutch traditional music
- [Free Music Archive](https://freemusicarchive.org/) selected electronic dance tracks

**WAIVE-Sampler** uses common techniques from machine listening to make measurements (called "features") of the audio, giving each sound a unique fingerprint and description of the audio content which can then be interpreted by other algorithms. At the same time, another method attempts to identify when percussive moments (called "onsets") occur using Complex Spectral Difference. The features are used to automatically identify appropriate moments in the Source audio that could be used for different drum hits, e.g. onsets that contain a broadband of frequency content can be shaped to make a snare sound.

For the Sample Map, the (log) Mel Spectrogram of the sample is measured, which computes a detailed representation of the frequency content of the audio. A simple, pretrained fully connected neural-net reads the measured features and outputs a 2D coordinate on where the audio sample should be placed on the map, e.g. such that kick drum sounds a grouped together, away from the hi-hat samples.

For the Tag cloud, we use an off-the-shelf word embedding model ([GloVe](https://nlp.stanford.edu/projects/glove/)) to produce a 2D representation (using TSNE dimensionality reduction) of all the tags in the database, such that semantically similar tags a clustered together to enable easier exploration of the available sources.
  
#### References
- [Jon Gillick, Adam Roberts, Jesse Engel, Douglas Eck, and David Bamman (2019) 'Learning to Groove with Inverse Sequence Transformations.'](https://magenta.tensorflow.org/datasets/groove), International Conference on Machine Learning 2019 (ICML). 
- [Gillick, J., Yang, J., Cella, C.-E. and Bamman, D. (2021) ‘Drumroll Please: Modeling Multi-Scale Rhythmic Gestures with Flexible Grids’](https://transactions.ismir.net/articles/10.5334/tismir.98#42-data-representations-for-comparison), Transactions of the International Society for Music Information Retrieval, 4(1), p. 156–166

### Licenses

- [DPF](https://github.com/DISTRHO/DPF?tab=ISC-1-ov-file) ISC license
- [ONNX Runtime](https://github.com/microsoft/onnxruntime) MIT
- [libsndfile](https://github.com/libsndfile/libsndfile?tab=LGPL-2.1-1-ov-file) LGPL-2.1 
- [VG5000 font](https://velvetyne.fr/fonts/vg5000/) SIL Open Font License, Version 1.1
- [Poppins Light font](https://fonts.google.com/specimen/Poppins) SIL Open Font License, Version 1.1
- [kissfft](https://github.com/mborgerding/kissfft) BSD-3-Clause
- [Gist](https://github.com/adamstark/Gist) GPL-3.0 license
- [libsamplerate](https://github.com/libsndfile/libsamplerate) BSD-3-Clause
- [nlohmann/json](https://github.com/nlohmann/json) MIT
- [TinyOSC](https://github.com/mhroth/tinyosc/tree/master) ISC license
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) zlib/libpng license
- [POCO](https://github.com/pocoproject/poco) Boost Software License
