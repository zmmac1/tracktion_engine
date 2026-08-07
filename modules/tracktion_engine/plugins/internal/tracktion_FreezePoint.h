/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class FreezePointPlugin    : public Plugin
{
public:
    // matches the global settings
    enum Position
    {
        beforeAllPlugins = 0,
        preFader,
        postFader
    };

    /** Temporarily unsolos all the tracks in an Edit. */
    struct ScopedTrackUnsoloer
    {
        ScopedTrackUnsoloer (Edit& e);
        ~ScopedTrackUnsoloer();

        Edit& edit;
        yup::BigInteger soloed, soloIsolated;
        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedTrackUnsoloer)
    };

    /** Temporarily solo isolates and unmutes some tracks. Useful for rendering */
    struct ScopedTrackSoloIsolator
    {
        ScopedTrackSoloIsolator (Edit& e, Track::Array&);
        ~ScopedTrackSoloIsolator();

        Edit& edit;
        Track::Array tracks;
        yup::BigInteger muted, notSoloIsolated;
        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedTrackSoloIsolator)
    };

    /** Temporarily disables a range of plugins in a track. */
    struct ScopedPluginDisabler
    {
        ScopedPluginDisabler (Track& track, yup::Range<int> pluginsToDisable);
        ~ScopedPluginDisabler();

    private:
        Track& track;
        yup::Range<int> indexes;
        yup::BigInteger enabled;

        void enablePlugins (bool enable);
        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedPluginDisabler)
    };

    /** Freezes a track if this plugin changes track. */
    struct ScopedTrackFreezer
    {
        ScopedTrackFreezer (FreezePointPlugin& o);
        ~ScopedTrackFreezer();

        EditItemID trackID;
        FreezePointPlugin& owner;
        bool wasFrozen;
        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedTrackFreezer)
    };

    static std::unique_ptr<ScopedTrackFreezer> createTrackFreezer (const Plugin::Ptr&);

    FreezePointPlugin (PluginCreationInfo);
    ~FreezePointPlugin() override;

    static const char* getPluginName()      { return NEEDS_TRANS("Freeze Point"); }
    static yup::ValueTree create();

    //==============================================================================
    static const char* xmlTypeName;

    void initialiseFully() override;
    yup::String getName() const override               { return TRANS("Freeze"); }
    yup::String getPluginType() override               { return xmlTypeName; }
    yup::String getTooltip() override                  { return TRANS("Track will freeze up to this plugin"); }
    yup::String getSelectableDescription() override    { return TRANS("Freeze Point Plugin"); }
    bool producesAudioWhenNoAudioInput() override       { return false; }
    bool canBeAddedToClip() override                    { return false; }
    bool canBeAddedToRack() override                    { return false; }
    bool canBeAddedToMaster() override                  { return false; }
    bool canBeDisabled() override                       { return false; }

    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;
    int getNumOutputChannelsGivenInputs (int numInputChannels) override     { return numInputChannels; }
    BusLayout getBusses() const override   { return BusLayout::singlePassThrough(); }

    bool isTrackFrozen() const;
    void freezeTrack (bool shouldBeFrozen);

private:
    //==============================================================================
    int lastFreezeIndex = -1;

    void updateTrackFreezeStatus();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FreezePointPlugin)
};

} // namespace tracktion::inline engine
