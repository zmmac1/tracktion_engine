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
class ClipTrack   : public Track,
                    public ClipOwner
{
public:
    ClipTrack (Edit&, const yup::ValueTree&, bool hasModifierList);
    ~ClipTrack() override;

    using Ptr   = yup::ReferenceCountedObjectPtr<ClipTrack>;

    /** @internal */
    void initialise() override;
    /** @internal */
    void flushStateToValueTree() override;

    //==============================================================================
    Clip* findClipForID (EditItemID) const override;

    //==============================================================================
    CollectionClip* getCollectionClip (int index) const noexcept;
    CollectionClip* getCollectionClip (Clip*) const;
    int getNumCollectionClips() const noexcept;
    int indexOfCollectionClip (CollectionClip*) const;
    int getIndexOfNextCollectionClipAt (TimePosition);
    CollectionClip* getNextCollectionClipAt (TimePosition);
    bool contains (CollectionClip*) const;
    void addCollectionClip (CollectionClip*);
    void removeCollectionClip (CollectionClip*);

    //==============================================================================
    int getNumTrackItems() const override;
    TrackItem* getTrackItem (int idx) const override;
    int indexOfTrackItem (TrackItem*) const override;
    int getIndexOfNextTrackItemAt (TimePosition) override;
    TrackItem* getNextTrackItemAt (TimePosition) override;

    //==============================================================================
    TimeDuration getLength() const;
    TimeDuration getLengthIncludingInputTracks() const;
    TimeRange getTotalRange() const;

    //==============================================================================
    bool addClip (const Clip::Ptr&);

    Clip* insertClipWithState (yup::ValueTree);

    Clip* insertClipWithState (const yup::ValueTree& stateToUse,
                               const yup::String& name, TrackItem::Type type,
                               ClipPosition position,
                               bool deleteExistingClips,
                               bool allowSpottingAdjustment);

    Clip* insertNewClip (TrackItem::Type, TimeRange position, SelectionManager* selectionManagerToSelectWith);
    Clip* insertNewClip (TrackItem::Type, const yup::String& name, TimeRange position, SelectionManager* selectionManagerToSelectWith);
    Clip* insertNewClip (TrackItem::Type, const yup::String& name, ClipPosition position, SelectionManager* selectionManagerToSelectWith);

    yup::ReferenceCountedObjectPtr<WaveAudioClip> insertWaveClip (const yup::String& name, const yup::File& sourceFile,
                                                                   ClipPosition position, bool deleteExistingClips);

    yup::ReferenceCountedObjectPtr<WaveAudioClip> insertWaveClip (const yup::String& name, ProjectItemRef sourceID,
                                                                   ClipPosition position, bool deleteExistingClips);

    yup::ReferenceCountedObjectPtr<MidiClip> insertMIDIClip (TimeRange position,
                                                              SelectionManager* selectionManagerToSelectWith);

    yup::ReferenceCountedObjectPtr<MidiClip> insertMIDIClip (const yup::String& name, TimeRange position,
                                                              SelectionManager* selectionManagerToSelectWith);

    yup::ReferenceCountedObjectPtr<EditClip> insertEditClip (TimeRange position, ProjectItemRef sourceID);

    void deleteRegion (TimeRange, SelectionManager*);
    void deleteRegionOfClip (Clip::Ptr, TimeRange, SelectionManager*);

    /** breaks a clip into 2 bits */
    Clip* splitClip (Clip&, TimePosition);

    /** split all clips at this time */
    void splitAt (TimePosition);

    /** Inserts space at the given time, moving clips whose center is after that point. */
    void insertSpace (TimePosition, TimeDuration);

    /** finds the next cut point */
    TimePosition getNextTimeOfInterest (TimePosition afterThisTime);
    TimePosition getPreviousTimeOfInterest (TimePosition beforeThisTime);

    bool containsPlugin (const Plugin*) const override;
    Plugin::Array getAllPlugins() const override;
    void sendMirrorUpdateToAllPlugins (Plugin&) const override;

    bool areAnyClipsUsingFile (const AudioFile&);
    bool containsAnyMIDIClips() const;

    /** @internal */
    yup::ValueTree& getClipOwnerState() override;
    /** @internal */
    EditItemID getClipOwnerID() override;
    /** @internal */
    Selectable* getClipOwnerSelectable() override;
    /** @internal */
    Edit& getClipOwnerEdit() override;
    /** @internal */
    void clipCreated (Clip&) override;
    /** @internal */
    void clipAddedOrRemoved() override;
    /** @internal */
    void clipOrderChanged() override;
    /** @internal */
    void clipPositionChanged() override;

protected:
    friend class Clip;

    yup::Array<TimePosition> findAllTimesOfInterest();

    struct CollectionClipList;
    friend struct CollectionClipList;
    std::unique_ptr<CollectionClipList> collectionClipList;

    void refreshTrackItems() const;

    mutable bool trackItemsDirty = false;
    mutable yup::Array<TrackItem*> trackItems;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipTrack)
};

} // namespace tracktion::inline engine
