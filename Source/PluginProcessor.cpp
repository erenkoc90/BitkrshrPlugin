/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BitkrshrAudioProcessor::BitkrshrAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    bitsParam = apvts.getRawParameterValue ("bits");
    holdParam = apvts.getRawParameterValue ("hold");
}

BitkrshrAudioProcessor::~BitkrshrAudioProcessor()
{
}
//======================================================
juce::AudioProcessorValueTreeState::ParameterLayout
BitkrshrAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // 1 ... 16 bit depth, default is 16 . in Arduino it was 8 ... 1.
    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "bits", 1 }, "Bit Depth", 1, 16, 16));

    // how often will i get a new sample: 1 ... 32
    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "hold", 1 }, "Sample Hold", 1, 32, 1));

    return layout;
}


//==============================================================================
const juce::String BitkrshrAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BitkrshrAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BitkrshrAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BitkrshrAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BitkrshrAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BitkrshrAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BitkrshrAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BitkrshrAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BitkrshrAudioProcessor::getProgramName (int index)
{
    return {};
}

void BitkrshrAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BitkrshrAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    held.fill (0.0f);
    holdCount = 0;
}

void BitkrshrAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BitkrshrAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}
#endif//


//=======================================================================================

void BitkrshrAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;   // super small numbers turn into 0, bc they can slow the CPU

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    // "read trimpots" = just like analogRead(A1) and analogRead(A2) in Arduino
    const int   bits   = (int) bitsParam->load();
    const int   holdN  = (int) holdParam->load();
    const float levels = std::pow (2.0f, (float) (bits - 1));

    float* const* data = buffer.getArrayOfWritePointers();

    for (int i = 0; i < numSamples; ++i)       // it s like loop() on Arduino
    {
        if (holdCount == 0)                    // take a new sample (~sample rate reduction)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                // analog to constrain(s, -128, 127) in Arduino!
                const float x = juce::jlimit (-1.0f, 1.0f, data[ch][i]);

                // bit depth reduction (round it)
                held[(size_t) ch] = std::round (x * levels) / levels;
            }
        }

        // analog to OCR2B = held on Arduino: write the held value to the exit
        for (int ch = 0; ch < numChannels; ++ch)
            data[ch][i] = held[(size_t) ch];

        if (++holdCount >= holdN)              
            holdCount = 0;
    }
}

//==============================================================================
bool BitkrshrAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BitkrshrAudioProcessor::createEditor()
{
    return new BitkrshrAudioProcessorEditor (*this);
}

//==============================================================================
void BitkrshrAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void BitkrshrAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitkrshrAudioProcessor();
}
