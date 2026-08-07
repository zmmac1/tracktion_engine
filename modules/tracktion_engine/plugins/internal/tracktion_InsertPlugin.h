/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class InsertPlugin  : public Plugin
{
public:
    enum DeviceType
    {
        noDevice,
        audioDevice,
        midiDevice
    };

    InsertPlugin (PluginCreationInfo);
    ~InsertPlugin() override;

    //==============================================================================
    static const char* getPluginName()          { return NEEDS_TRANS("Insert"); }
    static const char* xmlTypeName;
    static yup::ValueTree create();

    yup::String getName() const override;
    yup::String getPluginType() override;
    yup::String getShortName (int) override;
    double getLatencySeconds() override;
    void getChannelNames (yup::StringArray*, yup::StringArray*) override;
    bool takesMidiInput() override;
    bool canBeAddedToClip() override;
    bool needsConstantBufferSize() override;
    BusLayout getBusses() const override   { return BusLayout::singlePassThrough(); }

    void initialise (const PluginInitialisationInfo&) override;
    void initialiseWithoutStopping (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;
    yup::String getSelectableDescription() override;

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    DeviceType getSendDeviceType() const        { return sendDeviceType; }
    DeviceType getReturnDeviceType() const      { return returnDeviceType; }

    juce::CachedValue<yup::String> name, inputDevice, outputDevice;
    juce::CachedValue<double> manualAdjustMs;

    void updateDeviceTypes();

    /** Returns true if either the send or return types are audio. */
    bool hasAudio() const;

    /** Returns true if either the send or return types are MIDI. */
    bool hasMidi() const;

    static void getPossibleDeviceNames (Engine&,
                                        yup::StringArray& devices,
                                        yup::StringArray& aliases,
                                        yup::BigInteger& hasAudio,
                                        yup::BigInteger& hasMidi,
                                        bool forInput);

    /** @internal */
    int getLatencyNumSamples() const;

private:
    //==============================================================================
    std::atomic<int> latencyNumSamples { 0 };
    std::atomic<double> latencySeconds { 0.0 };
    DeviceType sendDeviceType = noDevice, returnDeviceType = noDevice;

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InsertPlugin)
};

} // namespace tracktion::inline engine
