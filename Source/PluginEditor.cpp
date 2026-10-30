#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    //==== colors ====//
    const juce::Colour bgColour     { 0xff3dcc21};   // green
    const juce::Colour accentColour { 0xff5a21cc };   // purple
    const juce::Colour textColour   { 0xffeaeaea };   // açık gri
}

BitkrshrAudioProcessorEditor::BitkrshrAudioProcessorEditor (BitkrshrAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
      bitsAttach (p.apvts, "bits", bitsSlider),
      holdAttach (p.apvts, "hold", holdSlider)
{
    
    auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::PressStart2P_ttf,
                                                             BinaryData::PressStart2P_ttfSize);
    if (typeface != nullptr)
        titleFont = juce::Font (juce::FontOptions (typeface).withHeight (32.0f));
    
    pixelLnf = std::make_unique<PixelLookAndFeel> (typeface);
    setLookAndFeel (pixelLnf.get());
    
    setupKnob (bitsSlider, bitsLabel, "Bit Depth");
    setupKnob (holdSlider, holdLabel, "Sample Hold");

    setSize (360, 280);
}

BitkrshrAudioProcessorEditor::~BitkrshrAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void BitkrshrAudioProcessorEditor::setupKnob (juce::Slider& s, juce::Label& l,
                                              const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    s.setColour (juce::Slider::rotarySliderFillColourId,    accentColour);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, accentColour.withAlpha (0.25f));
    s.setColour (juce::Slider::thumbColourId,               textColour);
    s.setColour (juce::Slider::textBoxTextColourId,         textColour);
    s.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, textColour);
    addAndMakeVisible (l);
}

void BitkrshrAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bgColour);

    // Plug-in name
    g.setColour (accentColour);
    g.setFont (titleFont);
    g.drawText ("Bit.krshr", getLocalBounds().removeFromTop (70),
                juce::Justification::centred);
}

void BitkrshrAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (70);          // space saved for the header
    area.reduce (20, 10);

    auto left = area.removeFromLeft (area.getWidth() / 2);
    bitsLabel.setBounds  (left.removeFromTop (24));
    bitsSlider.setBounds (left);

    holdLabel.setBounds  (area.removeFromTop (24));
    holdSlider.setBounds (area);
}
