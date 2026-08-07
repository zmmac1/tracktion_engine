/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class CompressorPlugin : public Plugin
{
public:
    CompressorPlugin (PluginCreationInfo);
    ~CompressorPlugin() override;

    //==============================================================================
    float getThreshold() const;
    void setThreshold (float);
    float getRatio() const;
    void setRatio (float);

    //==============================================================================
    static const char* getPluginName()      { return NEEDS_TRANS("Compressor/Limiter"); }
    static const char* xmlTypeName;

    yup::String getName() const override                               { return TRANS("Compressor"); }
    yup::String getPluginType() override                               { return xmlTypeName; }
    yup::String getShortName (int) override                            { return TRANS("Comp"); }
    int getNumOutputChannelsGivenInputs (int numInputChannels) override { return yup::jmin (numInputChannels, 2); }
    BusLayout getBusses() const override   { return BusLayout::singleStereoInOut(); }
    void getChannelNames (yup::StringArray*, yup::StringArray*) override;

    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;

    yup::String getSelectableDescription() override                    { return TRANS("Compressor/Limiter Plugin"); }

    ParameterWithStateValue thresholdGain, ratio, attackMs,
                            releaseMs, outputDb, sidechainDb;
    juce::CachedValue<bool> useSidechainTrigger;

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    static float getMinThreshold()      { return 0.01f; }
    static float getMaxThreshold()      { return 1.0f; }

private:
    double currentLevel = 0.0;
    float lastSamp = 0.0f;

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorPlugin)
};

} // namespace tracktion::inline engine
