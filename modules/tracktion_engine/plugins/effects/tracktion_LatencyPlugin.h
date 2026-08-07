/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** A test plugin that introduces latency to the incoming signal.
    This isn't added to the PluginManager by default as its main use is for
    internal testing.
*/
class LatencyPlugin  : public Plugin
{
public:
    LatencyPlugin (PluginCreationInfo);
    ~LatencyPlugin() override;

    //==============================================================================
    static const char* getPluginName()                      { return NEEDS_TRANS("Latency Tester"); }
    static const char* xmlTypeName;
    static yup::ValueTree create();

    yup::String getName() const override                   { return getPluginName(); }
    yup::String getPluginType() override                   { return xmlTypeName; }
    yup::String getSelectableDescription() override        { return getName(); }

    BusLayout getBusses() const override   { return BusLayout::singleStereoInOut(); }

    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;

    double getLatencySeconds() override                     { return latencyTimeSeconds.get(); }

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    //==============================================================================
    ConstrainedCachedValue<float> latencyTimeSeconds;
    juce::CachedValue<bool> applyLatency;

private:
    class DelayRegister;
    std::unique_ptr<DelayRegister> delayCompensator[2];
    LambdaTimer playbackRestartTimer;

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatencyPlugin)
};

} // namespace tracktion::inline engine
