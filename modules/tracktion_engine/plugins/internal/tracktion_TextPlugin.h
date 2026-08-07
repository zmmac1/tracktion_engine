/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** */
class TextPlugin   : public Plugin
{
public:
    TextPlugin (PluginCreationInfo);
    ~TextPlugin() override;

    static const char* getPluginName()                  { return NEEDS_TRANS("Text"); }
    static yup::ValueTree create();

    //==============================================================================
    static const char* xmlTypeName;

    bool canBeAddedToFolderTrack() override             { return true; }
    yup::String getName() const override               { return textTitle.get().isNotEmpty() ? textTitle : TRANS("Text Plugin"); }
    yup::String getPluginType() override               { return xmlTypeName; }
    void initialise (const PluginInitialisationInfo&) override {}
    void deinitialise() override                        {}
    void applyToBuffer (const PluginRenderContext&) override {}
    int getNumOutputChannelsGivenInputs (int numInputChannels) override     { return numInputChannels; }
    BusLayout getBusses() const override   { return BusLayout::singlePassThrough(); }
    bool producesAudioWhenNoAudioInput() override       { return false; }
    yup::String getSelectableDescription() override    { return TRANS("Text Plugin"); }

    juce::CachedValue<yup::String> textTitle, textBody;

private:
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextPlugin)
};

} // namespace tracktion::inline engine
