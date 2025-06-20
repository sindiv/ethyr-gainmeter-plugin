/*
    PluginProcessor.cpp
    
    Implementation of the GainMeter audio processor.
    
    This file contains the core audio processing logic, parameter management,
    and DAW integration code. All methods must maintain real-time safety
    in the processBlock() method.
    
    Author: Divij Singh
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Constructor - Initialize plugin with default settings
GainMeterAudioProcessor::GainMeterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Create main gain parameter with professional audio range
    // -60dB provides effective silence, +12dB allows useful boost without extremes
    addParameter(gainParameter = new juce::AudioParameterFloat(
        "gain",                                                 // Parameter ID for automation
        "Gain",                                                 // Display name
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),  // Min, max, step size
        0.0f                                                    // Default: unity gain (no change)
    ));
}

GainMeterAudioProcessor::~GainMeterAudioProcessor()
{
    // JUCE handles parameter cleanup automatically
}

//==============================================================================
// Plugin Metadata - Required by plugin formats

const juce::String GainMeterAudioProcessor::getName() const
{
    return JucePlugin_Name; // Defined in CMakeLists.txt configuration
}

bool GainMeterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false; // Audio-only processor, no MIDI needed
   #endif
}

bool GainMeterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false; // Does not generate MIDI events
   #endif
}

bool GainMeterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false; // Processes audio, not MIDI-only
   #endif
}

double GainMeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0; // Gain changes are instantaneous, no reverb tail
}

//==============================================================================
// Preset Management - Minimal implementation for format compliance

int GainMeterAudioProcessor::getNumPrograms()
{
    return 1; // Single program (current settings) - required minimum for some DAWs
}

int GainMeterAudioProcessor::getCurrentProgram()
{
    return 0; // Always program 0
}

void GainMeterAudioProcessor::setCurrentProgram (int index)
{
    // No multiple presets implemented
}

const juce::String GainMeterAudioProcessor::getProgramName (int index)
{
    return {}; // Empty string - no preset names
}

void GainMeterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    // Preset renaming not implemented
}

//==============================================================================
// Audio Processing Lifecycle

void GainMeterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize parameter smoothing to prevent audio clicks
    // 50ms smoothing time provides responsive feel while eliminating clicks
    gainSmoother.reset(sampleRate, 0.05);
    
    // Set initial target to current parameter value
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParameter->get()));
}

void GainMeterAudioProcessor::releaseResources()
{
    // Simple plugin - no large allocations to clean up
    // More complex plugins would free buffers, close files, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GainMeterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Support mono and stereo, but not surround configurations
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input and output channel counts must match (no channel conversion)
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
// Real-Time Audio Processing - THE CRITICAL METHOD

void GainMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Prevent denormalized numbers from causing CPU spikes
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any unused output channels to prevent noise
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Convert dB parameter to linear gain factor
    auto targetGain = juce::Decibels::decibelsToGain(gainParameter->get());
    gainSmoother.setTargetValue(targetGain);
    
    // Track peak level across all channels for metering
    float peakLevel = 0.0f;
    
    // Process each audio channel independently
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        // Process each sample with smooth gain interpolation
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Get smoothly interpolated gain value (prevents clicks)
            auto currentGain = gainSmoother.getNextValue();
            
            // Apply gain to audio sample
            channelData[sample] *= currentGain;
            
            // Track peak level for visual meter (absolute value for magnitude)
            peakLevel = juce::jmax(peakLevel, std::abs(channelData[sample]));
        }
    }
    
    // Update peak level for UI thread (thread-safe atomic operation)
    if (peakLevel > 0.0f)
        currentPeakLevel.store(juce::Decibels::gainToDecibels(peakLevel));
    else
        currentPeakLevel.store(-60.0f); // Represent silence as -60dB floor
}

//==============================================================================
// GUI Editor Management

bool GainMeterAudioProcessor::hasEditor() const
{
    return true; // Plugin provides visual interface
}

juce::AudioProcessorEditor* GainMeterAudioProcessor::createEditor()
{
    // Create editor instance, passing reference to this processor for parameter access
    return new GainMeterAudioProcessorEditor (*this);
}

//==============================================================================
// State Persistence - Project Save/Load Support

void GainMeterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Create hierarchical data structure for plugin state
    auto state = juce::ValueTree("GainMeterState");
    
    // Store current parameter value
    state.setProperty("gain", gainParameter->get(), nullptr);
    
    // Convert to XML format for cross-platform compatibility
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    
    // Serialize XML to binary format for storage
    copyXmlToBinary(*xml, destData);
}

void GainMeterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Deserialize binary data back to XML
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    // Validate and restore state with error checking
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("GainMeterState"))
        {
            auto state = juce::ValueTree::fromXml(*xmlState);
            
            // Restore parameter with fallback default
            *gainParameter = state.getProperty("gain", 0.0f);
        }
    }
}

//==============================================================================
// Plugin Factory Function - Required by plugin formats

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainMeterAudioProcessor();
}