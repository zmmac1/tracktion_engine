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
    The VCA plugin sits on a folder track to control the overall level of all the volume/pan
    plugins in its child tracks.
*/
class VCAPlugin   : public Plugin
{
public:
    VCAPlugin (PluginCreationInfo);
    ~VCAPlugin() override;

    static yup::ValueTree create();

    //==============================================================================
    float getSliderPos() const;
    void setVolumeDb (float dB);
    float getVolumeDb() const;
    void setSliderPos (float position);
    void muteOrUnmute();

    float updateAutomationStreamAndGetVolumeDb (TimePosition);

    //==============================================================================
    static const char* xmlTypeName;

    yup::String getName() const override               { return TRANS("VCA"); }
    yup::String getSelectableDescription() override    { return getName(); }
    yup::String getPluginType() override               { return xmlTypeName; }
    bool canBeAddedToClip() override                    { return false; }
    bool canBeAddedToRack() override                    { return false; }
    bool canBeAddedToFolderTrack()   override           { return true;  }
    bool canBeMoved() override;

    BusLayout getBusses() const override   { return BusLayout::singlePassThrough(); }
    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    juce::CachedValue<float> volumeValue;
    AutomatableParameter::Ptr volParam;

private:
    float lastVolumeBeforeMute = 0.0f;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VCAPlugin)
};

} // namespace tracktion::inline engine
