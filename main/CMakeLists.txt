# This example code is in the Public Domain (or CC0 licensed, at your option.)
# Unless required by applicable law or agreed to in writing, this
# software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, either express or implied.

idf_component_register(
    SRCS
      "main.cpp"
      "audio.cpp"
      "update.cpp"
      "adsr.cpp"
      "biquad.cpp"
      "mixer.cpp"
      "sequencer.cpp"
      "synth.cpp"
      "voice.cpp"
      "soundnode.cpp"
    INCLUDE_DIRS
        "." "include"
)
