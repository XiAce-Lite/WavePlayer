#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class WavePlayerEditor : public juce::AudioProcessorEditor,
    public juce::FileDragAndDropTarget,
    public juce::Timer
{
public:
    WavePlayerEditor(WavePlayerAudioProcessor&);
    ~WavePlayerEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    // File drag and drop
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // Timer for updating UI
    void timerCallback() override;
    std::unique_ptr<juce::FileChooser> fileChooser;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    WavePlayerAudioProcessor& audioProcessor;

    // UI Components
    juce::TextButton loadButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::Slider volumeSlider;
    juce::Slider positionSlider;
    juce::Label fileNameLabel;
    juce::Label timeLabel;

    // Waveform display (simple)
    juce::Rectangle<int> waveformArea;

    void updatePlayButtonText();
    void updateTimeDisplay();
    void formatTime(double seconds, juce::String& result);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavePlayerEditor)
};