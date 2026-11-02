#pragma once

#include <JuceHeader.h>

//==============================================================================
class BitkrshrAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    BitkrshrAudioProcessor();
    ~BitkrshrAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // parameters, just like the trimpots on Arduino
    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // pointers to reach the parameters safely from audio thread
    std::atomic<float>* bitsParam = nullptr;
    std::atomic<float>* holdParam = nullptr;

    // global variables: held and holdCount
    std::array<float, 2> held { 0.0f, 0.0f };   // for each channel, at most: stereo
    int holdCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitkrshrAudioProcessor)
};
