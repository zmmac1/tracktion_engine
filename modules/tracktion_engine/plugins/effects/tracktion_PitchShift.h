/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class PitchShiftPlugin : public Plugin
{
public:
    PitchShiftPlugin (Edit&, const yup::ValueTree&);
    PitchShiftPlugin (PluginCreationInfo);
    ~PitchShiftPlugin() override;

    static const char* getPluginName()          { return NEEDS_TRANS("Pitch Shifter"); }
    static yup::ValueTree create();

    //==============================================================================
    static const char* xmlTypeName;

    yup::String getName() const override       { return TRANS("Pitch Shifter"); }
    yup::String getPluginType() override       { return xmlTypeName; }
    yup::String getShortName (int) override    { return TRANS("Pitch"); }

    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    double getLatencySeconds() override;
    int getNumOutputChannelsGivenInputs (int numInputChannels) override  { return yup::jmin (numInputChannels, 2); }
    BusLayout getBusses() const override   { return BusLayout::singleStereoInOut(); }
    void applyToBuffer (const PluginRenderContext&) override;
    yup::String getSelectableDescription() override;
    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    juce::CachedValue<float> semitonesValue;
    juce::CachedValue<int> mode;
    juce::CachedValue<TimeStretcher::ElastiqueProOptions> elastiqueOptions;
    AutomatableParameter::Ptr semitones;

    static float getMaximumSemitones()   { return 2.0f * 12.0f; }

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShiftPlugin)
};

} // namespace tracktion::inline engine
