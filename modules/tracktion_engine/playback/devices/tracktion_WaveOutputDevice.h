/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** A (virtual) audio output device.

    There'll be one or more instances of these, each one representing a group of
    channels from a physical device.
*/
class WaveOutputDevice  : public OutputDevice
{
public:
    WaveOutputDevice (Engine&, const WaveDeviceDescription&);
    ~WaveOutputDevice() override;

    void resetToDefault();
    yup::String getDeviceTypeDescription() const override          { return NEEDS_TRANS("Wave Audio Output"); }
    void setEnabled (bool) override;
    const ChannelConfiguration& getChannels() const noexcept        { return deviceDescription.channels; }
    const yup::AudioChannelSet& getChannelSet() const noexcept     { return channelSet; }

    void reverseChannels (bool);
    bool isReversed() const         { return leftRightReversed; }

    void setDithered (bool);
    bool isDithered() const         { return ditheringEnabled; }

    /** @deprecated Use getChannels()[0].indexInDevice instead. */
    [[deprecated ("Use getChannels()[0].indexInDevice instead")]]
    int getLeftChannel() const;

    /** @deprecated Use getChannels()[1].indexInDevice instead. */
    [[deprecated ("Use getChannels()[1].indexInDevice instead")]]
    int getRightChannel() const;

    /** @deprecated Use getChannels().getNumChannels() == 2 instead. */
    [[deprecated ("Use getChannels().getNumChannels() == 2 instead")]]
    bool isStereoPair() const;

    /** @deprecated Use setChannelConfiguration() instead. */
    [[deprecated ("Use setChannelConfiguration() instead")]]
    void setStereoPair (bool);
    yup::PopupMenu createChannelGroupMenu (bool includeSetAllChannelsOptions);

    WaveOutputDeviceInstance* createInstance (EditPlaybackContext&);

    void updateLevelMeasurer (float* const* outputChannelData, int totalNumOutputChannels, int numSamples);

    /** Starts a gentle 1 second 440 Hz sine test tone playing through this device's channels. */
    void startTestTone();

    /** Called on the audio thread — mixes any in-progress test tone into the device's output channels. */
    void renderTestTone (float* const* outputChannelData, int totalNumOutputChannels, int numSamples);

    WaveDeviceDescription deviceDescription;
    LevelMeasurer levelMeasurer;

protected:
    yup::String openDevice() override;
    void closeDevice() override;

private:
    friend class DeviceManager;
    friend class WaveOutputDeviceInstance;

    ChannelConfiguration originalChannels;
    yup::AudioChannelSet channelSet;
    bool ditheringEnabled, leftRightReversed;
    choc::SmallVector<float*, 8> meterChannelPtrs;

    // Test tone state — pending* atomics are written by the message thread; the
    // remaining members are touched only on the audio thread.
    std::atomic<bool> testToneStartRequested { false };
    std::atomic<int>   testTonePendingTotalSamples { 0 };
    std::atomic<float> testTonePendingSampleRate   { 0.0f };
    int testToneSampleIndex = -1;
    int testToneTotalSamples = 0;
    choc::oscillator::Sine<float> testToneSine;

    void loadProps();
    void saveProps();
    void applyChannelOrdering();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveOutputDevice)
};

//==============================================================================
class WaveOutputDeviceInstance   : public OutputDeviceInstance
{
public:
    WaveOutputDeviceInstance (WaveOutputDevice&, EditPlaybackContext&);

    void prepareToPlay (double sampleRate, int blockSizeSamples);

protected:
    std::vector<Ditherer> ditherers;
    MidiMessageArray midiBuffer;
    yup::AudioBuffer<float> outputBuffer;

    WaveOutputDevice& getWaveOutput() const     { return static_cast<WaveOutputDevice&> (owner); }

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveOutputDeviceInstance)
};

} // namespace tracktion::inline engine
