/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class MidiPatchBayPlugin  : public Plugin
{
public:
    MidiPatchBayPlugin (PluginCreationInfo);
    ~MidiPatchBayPlugin() override;

    static const char* getPluginName()      { return NEEDS_TRANS("MIDI Patch Bay"); }
    static yup::ValueTree create();

    //==============================================================================
    static const char* xmlTypeName;

    yup::String getName() const override                                   { return TRANS("Patch Bay Plugin"); }
    yup::String getPluginType() override                                   { return xmlTypeName; }
    yup::String getShortName (int) override                                { return TRANS("MIDIPatch"); }
    bool canBeAddedToClip() override                                        { return false; }
    bool canBeAddedToRack() override                                        { return true; }
    BusLayout getBusses() const override   { return {}; }
    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;
    yup::String getSelectableDescription() override                        { return TRANS("MIDI Patch Bay Plugin"); }
    int getNumOutputChannelsGivenInputs (int) override                      { return 0; }
    void getChannelNames (yup::StringArray*, yup::StringArray*) override  {}

    //==============================================================================
    struct Mappings
    {
        char map[17] = {};
    };

    Mappings getMappings() const;
    void setMappings (const Mappings&);

    void resetMappings();
    void makeConnection (int srcChannel, int dstChannel);
    void sanityCheckMappings();
    void blockAllMappings();
    std::pair<int, int> getFirstMapping();

private:
    //==============================================================================
    enum { blockChannel = 0 };

    yup::CriticalSection mappingsLock;
    Mappings currentMappings;

    void valueTreeChanged() override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiPatchBayPlugin)
};

} // namespace tracktion::inline engine
