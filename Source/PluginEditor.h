#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PixelLookAndFeel : public juce::LookAndFeel_V4
{
public:
    explicit PixelLookAndFeel (juce::Typeface::Ptr tf) : typeface (tf) {}

    //======= all lables use this font ======//
    juce::Font getLabelFont (juce::Label& label) override
    {
        if (typeface != nullptr)
            return juce::Font (juce::FontOptions (typeface).withHeight (13.0f));

        return LookAndFeel_V4::getLabelFont (label);
    }

private:
    juce::Typeface::Ptr typeface;
};

class BitkrshrAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    BitkrshrAudioProcessorEditor (BitkrshrAudioProcessor&);
    ~BitkrshrAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void setupKnob (juce::Slider&, juce::Label&, const juce::String& name);

    BitkrshrAudioProcessor& processorRef;

    //====== Font of the header ====//
    juce::Font titleFont { juce::FontOptions { 32.0f } };
    std::unique_ptr<PixelLookAndFeel> pixelLnf;
    
    //====== First Slider and Label, then attachments ====//
    juce::Slider bitsSlider, holdSlider;
    juce::Label  bitsLabel,  holdLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment bitsAttach, holdAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitkrshrAudioProcessorEditor)
};
