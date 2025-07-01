/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// メンバ変数として追加
juce::File chatWavFile;
juce::int64 lastChatWavSize = -1;

//==============================================================================
WavePlayerEditor::WavePlayerEditor (WavePlayerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (500, 300);
    
    // Load button
    addAndMakeVisible(loadButton);
    loadButton.setButtonText("Load Wave File");
    loadButton.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select a Wave file to play...",
            juce::File{},
            "*.wav;*.aif;*.aiff;*.mp3"
        );

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    audioProcessor.loadWaveFile(file);
                    fileNameLabel.setText(file.getFileName(), juce::dontSendNotification);
                }
                fileChooser.reset(); // 後始末
            }
        );
    };
    
    // Play button
    addAndMakeVisible(playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this]() {
        if (audioProcessor.isPlaying())
            audioProcessor.stopPlayback();
        else
            audioProcessor.startPlayback();
    };
    
    // Stop button
    addAndMakeVisible(stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this]() {
        audioProcessor.stopPlayback();
        audioProcessor.setPlaybackPosition(0.0);
    };
    
    // Volume slider
    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(0.7);
    volumeSlider.onValueChange = [this]() {
        *audioProcessor.volumeParam = (float)volumeSlider.getValue();
    };
    
    // Position slider
    addAndMakeVisible(positionSlider);
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.setRange(0.0, 1.0);
    positionSlider.onValueChange = [this]() {
        double totalLength = audioProcessor.getTotalLength();
        if (totalLength > 0.0)
        {
            double newPosition = positionSlider.getValue() * totalLength;
            audioProcessor.setPlaybackPosition(newPosition);
        }
    };
    
    // File name label
    addAndMakeVisible(fileNameLabel);
    fileNameLabel.setText("No file loaded", juce::dontSendNotification);
    fileNameLabel.setJustificationType(juce::Justification::centred);
    
    // Time label
    addAndMakeVisible(timeLabel);
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    timeLabel.setJustificationType(juce::Justification::centred);

    // chat.wavのパスを取得
    chatWavFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("chat.wav");
    lastChatWavSize = chatWavFile.existsAsFile() ? chatWavFile.getSize() : -1;

    // Start timer for UI updates
    startTimer(50); // Update UI every 50ms
}

WavePlayerEditor::~WavePlayerEditor()
{
}

//==============================================================================
void WavePlayerEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
    
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Wave Player VST", getLocalBounds().removeFromTop(30), 
                      juce::Justification::centred, 1);
    
    // Draw waveform area background
    g.setColour(juce::Colours::black);
    g.fillRect(waveformArea);
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(waveformArea);
}

void WavePlayerEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(30); // Title space
    
    auto buttonArea = bounds.removeFromTop(40);
    loadButton.setBounds(buttonArea.removeFromLeft(120).reduced(5));
    playButton.setBounds(buttonArea.removeFromLeft(80).reduced(5));
    stopButton.setBounds(buttonArea.removeFromLeft(80).reduced(5));
    
    fileNameLabel.setBounds(bounds.removeFromTop(30).reduced(5));
    
    // Waveform area (placeholder for future waveform display)
    waveformArea = bounds.removeFromTop(80).reduced(10);
    
    auto sliderArea = bounds.removeFromTop(60);
    auto volumeArea = sliderArea.removeFromTop(30);
    volumeSlider.setBounds(volumeArea.removeFromLeft(200).reduced(5));
    
    positionSlider.setBounds(sliderArea.reduced(5));
    
    timeLabel.setBounds(bounds.removeFromTop(30).reduced(5));
}

bool WavePlayerEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& file : files)
    {
        if (file.contains(".wav") || file.contains(".aif") || file.contains(".mp3"))
            return true;
    }
    return false;
}

void WavePlayerEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
    for (auto& filename : files)
    {
        juce::File file(filename);
        if (file.exists())
        {
            audioProcessor.loadWaveFile(file);
            fileNameLabel.setText(file.getFileName(), juce::dontSendNotification);
            break; // Load only the first valid file
        }
    }
}

void WavePlayerEditor::timerCallback()
{
    updatePlayButtonText();
    updateTimeDisplay();

    // chat.wavの監視
    if (chatWavFile.existsAsFile())
    {
        juce::int64 newSize = chatWavFile.getSize();
        if (newSize > 0 && newSize != lastChatWavSize)
        {
            lastChatWavSize = newSize;
            audioProcessor.loadWaveFile(chatWavFile);
            audioProcessor.startPlayback();
        }
    }
    else
    {
        lastChatWavSize = -1;
    }

    // Update position slider
    double totalLength = audioProcessor.getTotalLength();
    if (totalLength > 0.0)
    {
        double currentPos = audioProcessor.getCurrentPosition();
        positionSlider.setValue(currentPos / totalLength, juce::dontSendNotification);
    }
}

void WavePlayerEditor::updatePlayButtonText()
{
    playButton.setButtonText(audioProcessor.isPlaying() ? "Pause" : "Play");
}

void WavePlayerEditor::updateTimeDisplay()
{
    double currentPos = audioProcessor.getCurrentPosition();
    double totalLength = audioProcessor.getTotalLength();
    
    juce::String currentTime, totalTime;
    formatTime(currentPos, currentTime);
    formatTime(totalLength, totalTime);
    
    timeLabel.setText(currentTime + " / " + totalTime, juce::dontSendNotification);
}

void WavePlayerEditor::formatTime(double seconds, juce::String& result)
{
    int mins = (int)(seconds / 60.0);
    int secs = (int)(seconds - mins * 60);
    result = juce::String::formatted("%02d:%02d", mins, secs);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavePlayerAudioProcessor();
}
