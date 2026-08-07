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
class FolderTrack  : public Track
{
public:
    FolderTrack (Edit&, const yup::ValueTree&);
    ~FolderTrack() override;

    using Ptr = yup::ReferenceCountedObjectPtr<FolderTrack>;

    //==============================================================================
    yup::String getSelectableDescription() override;
    bool isFolderTrack() const override;

    void initialise() override;

    void sanityCheckName() override;
    yup::String getName() const override;

    // not an index - values start from 1
    int getFolderTrackNumber() const noexcept;

    //==============================================================================
    bool isSubmixFolder() const;

    /** Returns the output if this track is a submix folder. */
    TrackOutput* getOutput() const noexcept;

    yup::Array<Track*> getInputTracks() const override;

    //==============================================================================
    bool isFrozen (FreezeType) const override;

    //==============================================================================
    float getVcaDb (TimePosition);
    VCAPlugin* getVCAPlugin();
    VolumeAndPanPlugin* getVolumePlugin();

    //==============================================================================
    void generateCollectionClips (SelectionManager&);
    CollectionClip* getCollectionClip (int index)  const noexcept;
    int getNumCollectionClips() const noexcept;
    int indexOfCollectionClip (CollectionClip*) const;
    int getIndexOfNextCollectionClipAt (TimePosition);
    CollectionClip* getNextCollectionClipAt (TimePosition);
    bool contains (CollectionClip*) const;

    //==============================================================================
    int getNumTrackItems() const override;
    TrackItem* getTrackItem (int idx) const override;
    int indexOfTrackItem (TrackItem*) const override;
    int getIndexOfNextTrackItemAt (TimePosition) override;
    TrackItem* getNextTrackItemAt (TimePosition) override;

    //==============================================================================
    bool isMuted (bool includeMutingByDestination) const override;
    bool isSolo (bool includeIndirectSolo) const override;
    bool isSoloIsolate (bool includeIndirectSolo) const override;

    void setMute (bool) override;
    void setSolo (bool) override;
    void setSoloIsolate (bool) override;

    //==============================================================================
    void setDirtyClips();

    bool canContainPlugin (Plugin*) const override;
    bool willAcceptPlugin (Plugin&);

private:
    friend class Edit;

    yup::ReferenceCountedArray<CollectionClip> collectionClips;
    juce::CachedValue<bool> muted, soloed, soloIsolated;
    bool dirtyClips = true;

    std::mutex pluginMutex;
    yup::ReferenceCountedObjectPtr<VCAPlugin> vcaPlugin;
    yup::ReferenceCountedObjectPtr<VolumeAndPanPlugin> volumePlugin;
    AsyncCaller pluginUpdater;

    void updatePlugins();
    TimeRange getClipExtendedBounds (Clip&);

    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override;
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override;
    void valueTreeChildOrderChanged (yup::ValueTree&, int, int) override;
};

} // namespace tracktion::inline engine
