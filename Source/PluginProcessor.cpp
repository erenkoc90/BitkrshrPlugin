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

    // 1 ... 16 bit, varsayılan 16 (neredeyse temiz). Arduino'da 8 ... 1 idi.
    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "bits", 1 }, "Bit Depth", 1, 16, 16));

    // Her kaç sample'da bir yeni sample alınacak: 1 ... 32
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
    juce::ignoreUnused (sampleRate, samplesPerBlock);  // şimdilik kullanmıyoruz
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
    juce::ScopedNoDenormals noDenormals;   // CPU'yu yavaşlatan çok küçük sayıları sıfırlar

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    // "Potları oku" = Arduino'daki analogRead(A1) ve analogRead(A2)
    const int   bits   = (int) bitsParam->load();
    const int   holdN  = (int) holdParam->load();
    const float levels = std::pow (2.0f, (float) (bits - 1));  // her yarım dalgadaki basamak sayısı

    float* const* data = buffer.getArrayOfWritePointers();

    for (int i = 0; i < numSamples; ++i)       // Arduino'da bu bir loop() turuydu
    {
        if (holdCount == 0)                    // yeni sample al (sample rate azaltma)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                // constrain(s, -128, 127) karşılığı:
                const float x = juce::jlimit (-1.0f, 1.0f, data[ch][i]);

                // bit derinliği azaltma: en yakın basamağa yuvarla
                held[(size_t) ch] = std::round (x * levels) / levels;
            }
        }

        // OCR2B = held karşılığı: tutulan değeri çıkışa yaz
        for (int ch = 0; ch < numChannels; ++ch)
            data[ch][i] = held[(size_t) ch];

        if (++holdCount >= holdN)              // aynı satır, Arduino kodundaki gibi
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
