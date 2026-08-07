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
*/
class Clipboard   : private yup::DeletedAtShutdown
{
public:
    //==============================================================================
    Clipboard();
    ~Clipboard();

    JUCE_DECLARE_SINGLETON (Clipboard, false)

    //==============================================================================
    struct ContentType
    {
        virtual ~ContentType();

        struct EditPastingOptions
        {
            EditPastingOptions (Edit&, EditInsertPoint&);
            EditPastingOptions (Edit&, EditInsertPoint&, SelectionManager*);

            Edit& edit;
            EditInsertPoint& insertPoint;
            Track::Ptr startTrack;
            TimePosition startTime;
            EditItemID targetClipOwnerID;
            SelectionManager* selectionManager = nullptr;
            bool silent = false;
            FileDragList::PreferredLayout preferredLayout = FileDragList::sameTrack;
            bool setTransportToEnd = false;
        };

        virtual bool pasteIntoEdit (Edit&, EditInsertPoint&, SelectionManager*) const;
        virtual bool pasteIntoEdit (const EditPastingOptions&) const;
    };

    struct ProjectItems  : public ContentType
    {
        ProjectItems();
        ~ProjectItems() override;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;
        bool pasteIntoProject (Project&) const;

        struct ItemInfo
        {
            ProjectItemRef itemID;
            TimeRange range;
        };

        std::vector<ItemInfo> itemIDs;
    };

    struct Clips  : public ContentType
    {
        Clips();
        ~Clips() override;

        bool pasteIntoEdit (Edit&, EditInsertPoint&, SelectionManager*) const override;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        bool pasteInsertingAtCursorPos (Edit&, EditInsertPoint&, SelectionManager&) const;
        bool pasteAfterSelected (Edit&, EditInsertPoint&, SelectionManager&) const;

        enum class AutomationLocked
        {
            no, /**< Don't copy autmation. */
            yes /**< Do copy autmation. */
        };

        void addClip (int trackOffset, const yup::ValueTree& state);
        void addSelectedClips (const SelectableList&, TimeRange, AutomationLocked);
        void addAutomation (const yup::Array<TrackSection>&, TimeRange);

        struct ClipInfo
        {
            yup::ValueTree state;
            int trackOffset = 0;
            std::optional<int> slotOffset;
            bool hasBeatTimes = false;
            BeatPosition startBeats;
            BeatDuration lengthBeats;
            BeatPosition offsetBeats;
            bool grouped = false;
        };

        std::vector<ClipInfo> clips;

        struct AutomationCurveSection
        {
            yup::String pluginName, paramID;
            int trackOffset = 0;
            std::vector<AutomationCurve::AutomationPoint> points;
            yup::Range<float> valueRange;
        };

        std::vector<AutomationCurveSection> automationCurves;
    };

    struct Scenes  : public ContentType
    {
        Scenes();
        ~Scenes() override;

        struct SceneInfo
        {
            yup::ValueTree state;
            std::vector<yup::ValueTree> clips;
        };

        std::vector<SceneInfo> scenes;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;
    };

    struct Tracks  : public ContentType
    {
        Tracks();
        ~Tracks() override;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        std::vector<yup::ValueTree> tracks;
    };

    struct TempoChanges  : public ContentType
    {
        TempoChanges (const TempoSequence&, TimeRange);
        ~TempoChanges() override;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        bool pasteTempoSequence (TempoSequence&, TimeRange targetRange) const;

        struct TempoChange
        {
            BeatPosition beat;
            double bpm;
            float curve;
        };

        std::vector<TempoChange> changes;
    };

    struct AutomationPoints  : public ContentType
    {
        AutomationPoints (const AutomationCurve&, yup::Range<float>, EditTimeRange, float defaultValue);
        AutomationPoints (AutomatableParameter&, const AutomationCurve&, TimeRange);
        /// Multi-track constructor: copies automation from all parameters on the given tracks within the time range.
        AutomationPoints (const yup::Array<Track*>&, TimeRange);
        /// Multi-parameter constructor: copies automation from specific parameters within the time range.
        AutomationPoints (const yup::Array<AutomatableParameter*>&, TimeRange);
        ~AutomationPoints() override;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        /// Multi-curve paste: pastes automation curves to target tracks by position offset.
        bool pasteIntoTracks (const yup::Array<Track*>& targetTracks, TimePosition pasteStartTime) const;

        bool pasteAutomationCurve (AutomationCurve& targetCurve, yup::Range<float> targetValueRange,
                                   float targetDefaultValue, std::optional<EditTimeRange> targetRange) const;

        bool pasteAutomationCurve (AutomatableParameter&, AutomationCurve&, TimeRange targetRange) const;

        /// Pastes a specific curve by index to the target parameter. Used when multi-curve content falls back to per-param paste.
        bool pasteAutomationCurveAtIndex (AutomatableParameter&, AutomationCurve&, TimeRange targetRange, size_t curveIndex) const;

        /// Internal structure representing a single curve's data.
        struct CurveSection
        {
            int trackOffset = 0;
            int paramIndex = 0;
            yup::String pluginName, paramID;
            std::vector<AutomationCurve::AutomationPoint> points;
            yup::Range<float> valueRange;
        };

        std::vector<CurveSection> curves;
        TimeDuration sourceDuration;
    };

    struct MIDIEvents  : public ContentType
    {
        MIDIEvents();
        ~MIDIEvents() override;

        std::pair<yup::Array<MidiNote*>, yup::Array<MidiControllerEvent*>> pasteIntoClip (MidiClip&,
                                                                                            const yup::Array<MidiNote*>& selectedNotes,
                                                                                            const yup::Array<MidiControllerEvent*>& selectedEvents,
                                                                                            TimePosition cursorPosition, const std::function<BeatPosition (BeatPosition)>& snapBeat,
                                                                                            int destController) const;


        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        std::vector<yup::ValueTree> notes;
        std::vector<yup::ValueTree> controllers;

    private:
        yup::Array<MidiNote*> pasteNotesIntoClip (MidiClip&, const yup::Array<MidiNote*>& selectedNotes,
                                                   TimePosition cursorPosition, const std::function<BeatPosition (BeatPosition)>& snapBeat) const;

        yup::Array<MidiControllerEvent*> pasteControllersIntoClip (MidiClip& clip,
                                                                    const yup::Array<MidiNote*>& selectedNotes,
                                                                    const yup::Array<MidiControllerEvent*>& selectedEvents,
                                                                    TimePosition cursorPosition, const std::function<BeatPosition (BeatPosition)>& snapBeat,
                                                                    int destController) const;
    };

    struct Pitches  : public ContentType
    {
        Pitches();
        ~Pitches() override;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        std::vector<yup::ValueTree> pitches;
    };

    struct TimeSigs  : public ContentType
    {
        TimeSigs();
        ~TimeSigs() override;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        std::vector<yup::ValueTree> timeSigs;
    };

    struct Plugins  : public ContentType
    {
        Plugins (const Plugin::Array&);
        ~Plugins() override;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        std::vector<yup::ValueTree> plugins;
        std::vector<std::pair<SafeSelectable<Edit>, yup::ValueTree>> rackTypes;
    };

    struct Takes  : public ContentType
    {
        Takes (const WaveCompManager&);
        ~Takes() override;

        bool pasteIntoClip (WaveAudioClip&) const;

        yup::ValueTree items;
    };

    struct Modifiers  : public ContentType
    {
        Modifiers();
        ~Modifiers() override;

        using ContentType::pasteIntoEdit;
        bool pasteIntoEdit (const EditPastingOptions&) const override;

        std::vector<yup::ValueTree> modifiers;
    };

    //==============================================================================
    void clear();
    bool isEmpty() const;

    void setContent (std::unique_ptr<ContentType> newContent);
    const ContentType* getContent() const;

    template <typename Type, typename... Args>
    void makeContent (Args&&... args)         { setContent (std::unique_ptr<Type> (new Type (std::forward<Args> (args)...))); }

    template <typename Type>
    const Type* getContentWithType() const    { return dynamic_cast<const Type*> (getContent()); }

    template <typename Type>
    bool hasContentWithType() const           { return getContentWithType<Type>() != nullptr; }

    //==============================================================================
    void addListener (yup::ChangeListener*);
    void removeListener (yup::ChangeListener*);

private:
    std::unique_ptr<ContentType> content;
    yup::ChangeBroadcaster broadcaster;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Clipboard)
};

AudioTrack* getOrInsertAudioTrackNearestIndex (Edit& edit, int trackIndex);

} // namespace tracktion::inline engine
