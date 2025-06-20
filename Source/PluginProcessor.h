/*
    PluginProcessor.h
    
    Main audio processor class for the Ethyr GainMeter plugin.
    
    This class handles real-time audio processing, parameter management, 
    and DAW integration. Follows JUCE AudioProcessor architecture with 
    emphasis on thread safety and professional audio practices.
    
    Author: Divij Singh
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

/**
 * Real-time gain control and peak metering audio processor.
 * 
 * Features:
 * - Real-time gain adjustment with parameter smoothing
 * - Peak level detection for visual metering
 * - Thread-safe communication between audio and UI threads
 * - Full DAW integration (automation, state persistence)
 * - Cross-platform VST3/AU support
 */
class GainMeterAudioProcessor : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    // Object Lifecycle
    GainMeterAudioProcessor();
    ~GainMeterAudioProcessor() override;

    //==============================================================================
    // Audio Processing Interface
    
    /** Called before audio processing starts. Initialize sample-rate dependent resources. */
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    
    /** Called when audio processing stops. Clean up resources. */
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    /** Determines which channel configurations this plugin supports. */
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    /** 
     * Main audio processing method - called in real-time audio thread.
     * 
     * Applies gain control with smooth parameter changes and tracks peak levels
     * for metering display. Must be real-time safe (no allocations, locks, or I/O).
     * 
     * @param buffer Audio buffer containing input samples (modified in-place)
     * @param midiMessages MIDI events (not used by this processor)
     */
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    //==============================================================================
    // Plugin Editor Interface
    
    /** Creates the plugin's GUI editor. */
    juce::AudioProcessorEditor* createEditor() override;
    
    /** Returns true as this plugin provides a visual interface. */
    bool hasEditor() const override;

    //==============================================================================
    // Plugin Metadata
    
    /** Returns the plugin name for display in DAW. */
    const juce::String getName() const override;

    /** Returns false - this plugin does not process MIDI input. */
    bool acceptsMidi() const override;
    
    /** Returns false - this plugin does not generate MIDI output. */
    bool producesMidi() const override;
    
    /** Returns false - this is an audio effect, not a MIDI-only effect. */
    bool isMidiEffect() const override;
    
    /** Returns 0.0 - gain changes have no tail (stop immediately). */
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Preset Management (Minimal Implementation)
    
    /** Returns 1 - single program (current state). */
    int getNumPrograms() override;
    
    /** Returns 0 - always using program 0. */
    int getCurrentProgram() override;
    
    /** No-op - single program implementation. */
    void setCurrentProgram (int index) override;
    
    /** Returns empty string - no named presets. */
    const juce::String getProgramName (int index) override;
    
    /** No-op - preset renaming not supported. */
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    // State Persistence
    
    /** Saves current plugin state to binary data for DAW project storage. */
    void getStateInformation (juce::MemoryBlock& destData) override;
    
    /** Restores plugin state from binary data when loading DAW projects. */
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Public Interface for GUI Communication
    
    /** 
     * Thread-safe access to current gain value for UI display.
     * @return Current gain setting in decibels
     */
    float getGainValue() const { return gainParameter->get(); }
    
    /** 
     * Thread-safe access to current peak level for meter display.
     * @return Current peak level in decibels (-60.0 to +12.0 range)
     */
    float getPeakLevel() const { return currentPeakLevel.load(); }
    
    /** 
     * Main gain parameter - exposed publicly for direct editor access.
     * Range: -60.0dB to +12.0dB, handles DAW automation and state persistence.
     */
    juce::AudioParameterFloat* gainParameter;

private:
    //==============================================================================
    // Thread-Safe Inter-Thread Communication
    
    /** 
     * Current peak level for metering display.
     * Updated by audio thread, read by UI thread. Atomic ensures thread safety.
     */
    std::atomic<float> currentPeakLevel { 0.0f };
    
    /** 
     * Smooths gain parameter changes to prevent audio clicks.
     * Provides gradual transitions when user adjusts gain control.
     */
    juce::LinearSmoothedValue<float> gainSmoother;
    
    //==============================================================================
    // Development Safety
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainMeterAudioProcessor)
};