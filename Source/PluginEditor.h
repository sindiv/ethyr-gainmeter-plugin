/*
    PluginEditor.h
    
    GUI components for the GainMeter plugin interface.
    
    Implements a professional audio plugin interface with real-time peak metering
    and responsive gain control. Uses JUCE component architecture with proper
    separation between audio processing and UI threads.
    
    Author: Divij Singh
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Real-time peak meter component with professional audio styling.
 * 
 * Features:
 * - 30 FPS update rate for smooth animation
 * - Color-coded level indication (green/yellow/red)
 * - dB scale with numeric readout
 * - Thread-safe communication with audio processor
 */
class PeakMeter : public juce::Component, private juce::Timer
{
public:
    /**
     * Constructor initializes meter with reference to audio processor.
     * @param processor Reference to audio processor for level data access
     */
    explicit PeakMeter(GainMeterAudioProcessor& processor) : audioProcessor(processor)
    {
        // 30 FPS provides smooth visual updates without excessive CPU usage
        startTimerHz(30);
    }
    
    /**
     * Custom paint method renders the peak meter with professional styling.
     * Implements standard audio meter conventions with color coding.
     */
    void paint(juce::Graphics& g) override
    {
        // Draw meter background and border
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::darkgrey);
        g.drawRect(getLocalBounds(), 2);
        
        // Get current peak level from audio processor (thread-safe)
        auto levelDb = audioProcessor.getPeakLevel();
        
        // Normalize dB range (-60 to +12) to 0.0-1.0 for rendering
        auto normalizedLevel = juce::jmap(levelDb, -60.0f, 12.0f, 0.0f, 1.0f);
        normalizedLevel = juce::jlimit(0.0f, 1.0f, normalizedLevel);
        
        // Calculate meter bar dimensions
        auto meterBounds = getLocalBounds().reduced(4);
        auto meterHeight = static_cast<int>(meterBounds.getHeight() * normalizedLevel);
        
        // Draw level bar with professional color coding
        if (meterHeight > 0)
        {
            auto meterRect = meterBounds.removeFromBottom(meterHeight);
            
            // Industry standard color zones:
            // Green: -∞ to -12dB (safe operating level)
            // Yellow: -12dB to -3dB (caution zone)  
            // Red: -3dB to 0dB+ (approaching/exceeding digital full scale)
            juce::Colour meterColor;
            if (levelDb < -12.0f)
                meterColor = juce::Colours::green;
            else if (levelDb < -3.0f)
                meterColor = juce::Colours::yellow;
            else
                meterColor = juce::Colours::red;
                
            g.setColour(meterColor);
            g.fillRect(meterRect);
        }
        
        // Draw numeric level display
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        auto levelText = juce::String(levelDb, 1) + " dB";
        g.drawText(levelText, getLocalBounds().removeFromBottom(20), 
                  juce::Justification::centred, true);
    }
    
private:
    GainMeterAudioProcessor& audioProcessor;
    
    /**
     * Timer callback triggers meter redraws for smooth animation.
     * Called 30 times per second for responsive visual feedback.
     */
    void timerCallback() override
    {
        repaint(); // Request repaint on next graphics update cycle
    }
};

//==============================================================================
/**
 * Main plugin editor window containing gain control and peak meter.
 * 
 * Implements professional plugin UI patterns:
 * - Real-time parameter visualization
 * - Responsive user controls with immediate feedback
 * - Thread-safe communication with audio processor
 * - Professional visual styling
 */
class GainMeterAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Slider::Listener
{
public:
    /**
     * Constructor sets up complete user interface.
     * @param processor Reference to audio processor for parameter access
     */
    explicit GainMeterAudioProcessorEditor (GainMeterAudioProcessor& processor);
    
    /**
     * Destructor handles cleanup of UI resources.
     */
    ~GainMeterAudioProcessorEditor() override;

    //==============================================================================
    // Component Interface
    
    /**
     * Renders plugin background and static visual elements.
     * @param g Graphics context for drawing operations
     */
    void paint (juce::Graphics& g) override;
    
    /**
     * Positions child components when editor window is resized.
     * Implements responsive layout that adapts to different window sizes.
     */
    void resized() override;
    
private:
    //==============================================================================
    // Control Event Handling
    
    /**
     * Handles slider value changes and updates audio processor parameters.
     * Provides immediate response to user control movements.
     * @param slider Pointer to the slider that changed
     */
    void sliderValueChanged(juce::Slider* slider) override;
    
    //==============================================================================
    // Component References
    
    /** Reference to audio processor for parameter access and meter data */
    GainMeterAudioProcessor& audioProcessor;
    
    //==============================================================================
    // UI Components
    
    /** Main gain control slider with dB scaling */
    juce::Slider gainSlider;
    
    /** Text label for gain control */
    juce::Label gainLabel;
    
    /** Real-time peak level meter display */
    std::unique_ptr<PeakMeter> peakMeter;

    //==============================================================================
    // Development Safety
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainMeterAudioProcessorEditor)
};