/*
    PluginEditor.cpp
    
    Implementation of the GainMeter plugin user interface.
    
    Creates a professional audio plugin interface with real-time visual feedback,
    responsive controls, and industry-standard styling. Maintains thread-safe
    communication between UI and audio processing threads.
    
    Author: Divij Singh
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Editor Constructor - Complete UI Setup

GainMeterAudioProcessorEditor::GainMeterAudioProcessorEditor (GainMeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //==============================================================================
    // Gain Slider Configuration
    
    // Set up vertical slider style (industry standard for gain controls)
    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    
    // Configure range matching the audio processor parameter
    // -60dB minimum provides effective silence
    // +12dB maximum allows useful boost without extreme levels
    // 0.1dB steps provide fine control resolution
    gainSlider.setRange(-60.0, 12.0, 0.1);
    
    // Initialize slider with current processor value (important for automation)
    gainSlider.setValue(audioProcessor.getGainValue());
    
    // Configure text display box for numeric feedback
    gainSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,  // Position below slider track
        false,                       // Editable (user can type values)
        80,                          // Width in pixels
        20                           // Height in pixels
    );
    
    // Add dB suffix for clear units indication
    gainSlider.setTextValueSuffix(" dB");
    
    // Register for slider change notifications
    gainSlider.addListener(this);
    addAndMakeVisible(gainSlider);
    
    //==============================================================================
    // Gain Label Configuration
    
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    
    // Automatically position label above slider
    gainLabel.attachToComponent(&gainSlider, false);
    addAndMakeVisible(gainLabel);
    
    //==============================================================================
    // Peak Meter Setup
    
    // Create meter component with processor reference for data access
    peakMeter = std::make_unique<PeakMeter>(audioProcessor);
    addAndMakeVisible(*peakMeter);
    
    //==============================================================================
    // Window Configuration
    
    // Set reasonable default size for plugin window
    // Dimensions chosen to accommodate controls with comfortable spacing
    setSize (300, 400);
}

GainMeterAudioProcessorEditor::~GainMeterAudioProcessorEditor()
{
    // std::unique_ptr handles automatic cleanup of peakMeter
    // JUCE handles cleanup of slider and label components
}

//==============================================================================
// Background Rendering

void GainMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    //==============================================================================
    // Professional Gradient Background
    
    // Base color fill for compatibility
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Create subtle gradient for depth and visual appeal
    juce::ColourGradient gradient(
        juce::Colour(0xff3a3a3a), 0, 0,              // Lighter shade at top
        juce::Colour(0xff1a1a1a), 0, getHeight(),    // Darker shade at bottom
        false                                         // Linear (not radial) gradient
    );
    
    g.setGradientFill(gradient);
    g.fillAll();
    
    //==============================================================================
    // Plugin Branding
    
    // Draw plugin title with professional typography
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    
    // Center title in reserved header area
    g.drawText("Gain Meter", 
              getLocalBounds().removeFromTop(40),
              juce::Justification::centred, 
              true);
}

//==============================================================================
// Component Layout Management

void GainMeterAudioProcessorEditor::resized()
{
    //==============================================================================
    // Responsive Layout Implementation
    
    auto bounds = getLocalBounds();
    
    // Reserve space for title header
    bounds.removeFromTop(40);
    
    // Add comfortable margin around controls
    bounds.reduce(20, 10);
    
    //==============================================================================
    // Horizontal Split Layout
    
    // Divide available space between gain control and meter display
    auto gainSection = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto meterSection = bounds; // Remaining area for meter
    
    //==============================================================================
    // Position Gain Controls
    
    // Reserve space for automatically positioned label
    gainSection.removeFromTop(25);
    
    // Position slider with comfortable margins
    gainSlider.setBounds(gainSection.reduced(10));
    
    //==============================================================================
    // Position Peak Meter
    
    // Meter takes remaining space with margins
    peakMeter->setBounds(meterSection.reduced(10));
}

//==============================================================================
// User Interaction Handling

void GainMeterAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // Handle gain slider changes
    if (slider == &gainSlider)
    {
        //==============================================================================
        // Thread-Safe Parameter Update
        
        // Update processor parameter (thread-safe via JUCE AudioParameterFloat)
        // Automatic conversion from double (UI precision) to float (audio precision)
        *audioProcessor.gainParameter = static_cast<float>(gainSlider.getValue());
        
        /*
         * Parameter Update Flow:
         * 1. User moves slider → sliderValueChanged() called
         * 2. We update AudioParameterFloat → thread-safe communication initiated
         * 3. Audio thread reads new value in processBlock() → applies smoothed gain
         * 4. Peak meter automatically reflects new levels → visual feedback complete
         * 5. DAW can record parameter changes for automation
         * 6. Value automatically saved with project state
         */
    }
    
    // Extensible pattern for additional controls:
    // else if (slider == &otherSlider) { ... }
}