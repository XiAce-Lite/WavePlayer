#include "PluginProcessor.h"
#include "PluginEditor.h"

WavePlayerAudioProcessor::WavePlayerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    resamplingSource(&transportSource, false, 2)
{
    // Register audio formats
    formatManager.registerBasicFormats();

    // Add parameters
    addParameter(volumeParam = new juce::AudioParameterFloat("volume", "Volume", 0.0f, 1.0f, 0.7f));
    addParameter(playParam = new juce::AudioParameterBool("play", "Play", false));
}

WavePlayerAudioProcessor::~WavePlayerAudioProcessor()
{
    transportSource.setSource(nullptr);
}

//==============================================================================
const juce::String WavePlayerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WavePlayerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool WavePlayerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool WavePlayerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double WavePlayerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WavePlayerAudioProcessor::getNumPrograms()
{
    return 1;
}

int WavePlayerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WavePlayerAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String WavePlayerAudioProcessor::getProgramName(int index)
{
    return {};
}

void WavePlayerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void WavePlayerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    resamplingSource.prepareToPlay(samplesPerBlock, sampleRate);

    // ファイルがロードされていれば、ここでリサンプリング比率を設定
    if (readerSource != nullptr && fileSampleRate > 0.0)
    {
        double ratio = fileSampleRate / sampleRate;
        resamplingSource.setResamplingRatio(ratio);
    }
}

void WavePlayerAudioProcessor::releaseResources()
{
    transportSource.releaseResources();
    resamplingSource.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WavePlayerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void WavePlayerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    buffer.clear();

    if (!playing || waveBuffer.getNumSamples() == 0)
        return;

    int numChannels = juce::jmin(buffer.getNumChannels(), waveBuffer.getNumChannels());
    int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        int64_t pos = playPosition;
        int samplesToCopy = juce::jmin((int64_t)numSamples, (int64_t)waveBuffer.getNumSamples() - pos);

        if (samplesToCopy > 0)
            buffer.copyFrom(ch, 0, waveBuffer, ch, (int)pos, samplesToCopy);

        if (samplesToCopy < numSamples)
            buffer.clear(ch, samplesToCopy, numSamples - samplesToCopy);
    }

    playPosition += numSamples;

    if (playPosition >= waveBuffer.getNumSamples())
    {
        playing = false;
        playPosition = 0;
    }

    buffer.applyGain(volumeParam->get());
}

//==============================================================================
bool WavePlayerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* WavePlayerAudioProcessor::createEditor()
{
    return new WavePlayerEditor(*this);
}

//==============================================================================
void WavePlayerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save the current file path and playback state
    juce::ValueTree state("WavePlayerState");
    state.setProperty("fileName", currentFileName, nullptr);
    state.setProperty("volume", volumeParam->get(), nullptr);

    juce::MemoryOutputStream stream(destData, false);
    state.writeToStream(stream);
}

void WavePlayerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore the file and state
    juce::ValueTree state = juce::ValueTree::readFromData(data, sizeInBytes);
    if (state.isValid())
    {
        currentFileName = state.getProperty("fileName", "");
        *volumeParam = state.getProperty("volume", 0.7f);

        if (currentFileName.isNotEmpty())
        {
            juce::File file(currentFileName);
            if (file.exists())
                loadWaveFile(file);
        }
    }
}

//==============================================================================
void WavePlayerAudioProcessor::loadWaveFile(const juce::File& file)
{
    if (auto* reader = formatManager.createReaderFor(file))
    {
        waveBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&waveBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
        waveSampleRate = reader->sampleRate;
        playPosition = 0;
        delete reader; // ここでファイルは閉じられる
    }
}

void WavePlayerAudioProcessor::startPlayback() 
{ 
    playPosition = 0;
    playing = true;
}
void WavePlayerAudioProcessor::stopPlayback() 
{ 
    playing = false; 
    playPosition = 0; 
}

void WavePlayerAudioProcessor::setPlaybackPosition(double seconds)
{
    playPosition = (int64_t)(seconds * waveSampleRate);
}

double WavePlayerAudioProcessor::getCurrentPosition() const
{
    return (double)playPosition / waveSampleRate;
}

double WavePlayerAudioProcessor::getTotalLength() const
{
    return (double)waveBuffer.getNumSamples() / waveSampleRate;
}