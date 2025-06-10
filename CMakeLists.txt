cmake_minimum_required(VERSION 3.15)

project(EthyrGainMeterPlugin VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# Set path to your JUCE folder
set(JUCE_DIR "/Users/divijsingh/JUCE")

# Add JUCE
add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(EthyrGainMeter
    COMPANY_NAME "Ethyr Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE ETHR
    PLUGIN_CODE GnMt
    FORMATS VST3 AU
    PRODUCT_NAME "Ethyr GainMeter"
)

target_sources(EthyrGainMeter PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_definitions(EthyrGainMeter PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(EthyrGainMeter PRIVATE
    juce::juce_audio_utils
)
