/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/**
    The built-in Tracktion volume/pan plugin.
*/
class VolumeAndPanPlugin  : public Plugin
{
public:
    VolumeAndPanPlugin (Edit&, const yup::ValueTree&, bool isMasterVolumeNode);
    VolumeAndPanPlugin (PluginCreationInfo, bool isMasterVolumeNode = false);
    ~VolumeAndPanPlugin() override;

    using Ptr = yup::ReferenceCountedObjectPtr<VolumeAndPanPlugin>;

    static const char* getPluginName()                      { return NEEDS_TRANS("Volume and Pan"); }
    static yup::ValueTree create();

    bool isMasterVolAndPan()                                { return isMasterVolume; }
    bool canBeAddedToRack() override                        { return ! isMasterVolume; }
    bool canBeMoved() override                              { return ! isMasterVolume; }

    //==============================================================================
    float getVolumeDb() const;
    float getSliderPos() const                              { return volParam->getCurrentValue(); }
    void setVolumeDb (float vol);
    void setSliderPos (float pos);

    float getPan() const                                    { return panParam->getCurrentValue(); }
    void setPan (float pan);

    void setPanLaw (PanLaw newPanLaw)                       { panLaw = (int) newPanLaw; }
    PanLaw getPanLaw() const noexcept;

    void setAppliedToMidiVolumes (bool);
    bool isAppliedToMidiVolumes() const                     { return applyToMidi; }

    void muteOrUnmute();

    bool shouldMeasureCpuUsage() const noexcept final       { return false; }

    //==============================================================================
    static const char* xmlTypeName;

    yup::String getName() const override                   { return TRANS("Volume & Pan Plugin"); }
    yup::String getPluginType() override                   { return xmlTypeName; }
    yup::String getShortName (int) override                { return "VolPan"; }
    yup::String getSelectableDescription() override        { return getName(); }

    void initialise (const PluginInitialisationInfo&) override;
    void initialiseWithoutStopping (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;
    int getNumOutputChannelsGivenInputs (int numInputs) override    { return numInputs; }
    BusLayout getBusses() const override   { return BusLayout::singleInOut (ChannelConfiguration::stereo(), ChannelConfiguration::none()); }

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    //==============================================================================
    juce::CachedValue<float> volume, pan;
    juce::CachedValue<bool> applyToMidi, ignoreVca, polarity;
    juce::CachedValue<int> panLaw;

    // NB the units used here are slider position
    AutomatableParameter::Ptr volParam, panParam;

    /// This is the time over which volume changes will be ramped, to avoid
    /// zipper noise.
    double smoothingRampTimeSeconds = 0.015;

private:
    float lastVolumeBeforeMute = 0.0f;
    yup::SmoothedValue<float> smoothedGainL, smoothedGainR, smoothedGain;

    RealTimeSpinLock vcaTrackLock;
    yup::ReferenceCountedObjectPtr<AudioTrack> vcaTrack;
    const bool isMasterVolume = false;

    void setSmoothedValueTargets (TimePosition, bool);
    void refreshVCATrack();
    float getVCAPosDelta (TimePosition);

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VolumeAndPanPlugin)
};

} // namespace tracktion::inline engine
