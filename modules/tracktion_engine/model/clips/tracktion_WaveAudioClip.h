/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

//==============================================================================
/**
    An audio clip that uses an audio file as its source.
*/
class WaveAudioClip  : public AudioClipBase
{
public:
    /** Creates a WaveAudioClip from a given state. @see ClipTrack::insertWaveClip. */
    WaveAudioClip (const yup::ValueTree&, EditItemID, ClipOwner&);

    /** Destructor. */
    ~WaveAudioClip() override;

    using Ptr = yup::ReferenceCountedObjectPtr<WaveAudioClip>;

    //==============================================================================
    /** Adds a new take with the ProjectItemRef as the source. */
    void addTake (ProjectItemRef);

    /** Adds a new take with the given file as the source. */
    void addTake (const yup::File&);

    /** Deletes all but the current takes.
        @param deleteSourceFiles    If true, also deletes the source files
    */
    void deleteAllUnusedTakes (bool deleteSourceFiles);

    /** Deletes all but the current takes but shows a confirmation dialog first.
        @param deleteSourceFiles    If true, also deletes the source files
    */
    void deleteAllUnusedTakesConfirmingWithUser (bool deleteSourceFiles);

    /** Returns the WaveCompManager for this clip. */
    WaveCompManager& getCompManager();

    //==============================================================================
    /** @internal */
    bool needsRender() const override;
    /** @internal */
    RenderManager::Job::Ptr getRenderJob (const AudioFile& destFile) override;
    /** @internal */
    yup::String getRenderMessage() override;
    /** @internal */
    void renderComplete() override;

    /** @internal */
    bool isUsingFile (const AudioFile&) override;

    //==============================================================================
    /** @internal */
    void initialise() override;
    /** @internal */
    void cloneFrom (Clip*) override;

    //==============================================================================
    /** @internal */
    yup::String getSelectableDescription() override;

    /** @internal */
    bool isMidi() const override                                { return false; }
    /** @internal */
    bool usesSourceFile() const override                        { return true; }

    /** @internal */
    TimeDuration getSourceLength() const override;
    /** @internal */
    void sourceMediaChanged() override;

    /** @internal */
    yup::File getOriginalFile() const override;
    /** @internal */
    HashCode getHash() const override;

    /** @internal */
    void setLoopDefaults() override;

    //==============================================================================
    /** @internal */
    yup::StringArray getTakeDescriptions() const override;
    /** @internal */
    bool hasAnyTakes() const override                           { return getTakesTree().getNumChildren() > 0; }
    /** @internal */
    int getNumTakes (bool includeComps) override;
    /** @internal */
    yup::Array<ProjectItemRef> getTakes() const override;
    /** @internal */
    void clearTakes() override;
    /** @internal */
    int getCurrentTake() const override;
    /** @internal */
    void setCurrentTake (int takeIndex) override;
    /** @internal */
    bool isCurrentTakeComp() override;
    /** @internal */
    Clip::Array unpackTakes (bool toNewTracks) override;

    /** @internal */
    void reassignReferencedItem (const ReferencedItem&, ProjectItemRef newRef, double newStartTime) override;

private:
    //==============================================================================
    mutable TimeDuration sourceLength;
    WaveCompManager::Ptr compManager;

    static constexpr int takeIndexNeedsUpdating = -2;
    mutable int currentTakeIndex = takeIndexNeedsUpdating;

    yup::ValueTree getTakesTree() const                        { return state.getChildWithName (IDs::TAKES); }
    void invalidateCurrentTake() noexcept;
    void invalidateCurrentTake (const yup::ValueTree&) noexcept;

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override;
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override;
    void valueTreeChildOrderChanged (yup::ValueTree&, int, int) override;

    //==============================================================================
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveAudioClip)
};

} // namespace tracktion::inline engine
