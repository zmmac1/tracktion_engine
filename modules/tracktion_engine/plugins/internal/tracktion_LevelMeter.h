/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class LevelMeterPlugin  : public Plugin,
                          private yup::Timer
{
public:
    LevelMeterPlugin (PluginCreationInfo);
    ~LevelMeterPlugin() override;

    static yup::ValueTree create();

    //==============================================================================
    static const char* getPluginName()              { return NEEDS_TRANS("Level Meter"); }
    static const char* xmlTypeName;

    yup::String getName() const override               { return TRANS("Level Meter"); }
    yup::String getPluginType() override               { return xmlTypeName; }
    yup::String getShortName (int) override            { return "Meter"; }
    yup::String getTooltip() override                  { return TRANS("Level meter plugin") + "$levelmeterplugin"; }
    bool canBeDisabled() override                       { return false; }
    bool shouldMeasureCpuUsage() const noexcept final   { return false; }

    int getNumOutputChannelsGivenInputs (int numInputChannels) override     { return numInputChannels; }
    BusLayout getBusses() const override   { return BusLayout::singlePassThrough(); }

    yup::String getSelectableDescription() override                        { return TRANS("Level Meter Plugin"); }
    void initialise (const PluginInitialisationInfo&) override;
    void initialiseWithoutStopping (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;
    void timerCallback() override;

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    //==============================================================================
    LevelMeasurer measurer;

    juce::CachedValue<bool> showMidiActivity;

private:
    int controllerTrack = -1;
    LevelMeasurer::Client controllerLevelClient;
    std::vector<float> controllerGainsBuffer;

   #if TRACKTION_UNIT_TESTS
    friend struct LevelMeterPluginTestAccess;
   #endif

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeterPlugin)
};

} // namespace tracktion::inline engine
