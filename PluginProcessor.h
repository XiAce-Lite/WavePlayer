#pragma once

#include <JuceHeader.h>

//==============================================================================
class WavePlayerAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    WavePlayerAudioProcessor();
    ~WavePlayerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Wave file functions
    void loadWaveFile(const juce::File& file);
    void startPlayback();
    void stopPlayback();
    void setPlaybackPosition(double positionInSeconds);
    bool isPlaying() const { return playing; }
    double getCurrentPosition() const;
    double getTotalLength() const;

    // Parameters
    juce::AudioParameterFloat* volumeParam;
    juce::AudioParameterBool* playParam;

private:
    //==============================================================================
    // Audio file handling
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> waveBuffer;
    double waveSampleRate = 44100.0;
    int64_t playPosition = 0;
	bool playing = false;

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resamplingSource;

    // File info
    juce::String currentFileName;
    double sampleRateRatio = 1.0;
    double fileSampleRate = 0.0; // 追加: ロードしたファイルのサンプルレートを保持

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavePlayerAudioProcessor)
};