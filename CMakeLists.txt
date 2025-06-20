cmake_minimum_required(VERSION 3.15)

project(GainMeterPlugin VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# Set path to your JUCE folder
set(JUCE_DIR "/Users/divijsingh/JUCE")

# Verify JUCE exists
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE not found at ${JUCE_DIR}")
endif()

# Add JUCE
add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(GainMeter
    COMPANY_NAME "Esoteryca"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE ETHR
    PLUGIN_CODE GnMt
    FORMATS VST3 AU
    PRODUCT_NAME "GainMeter"
)

target_sources(GainMeter PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_definitions(GainMeter PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(GainMeter PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
)