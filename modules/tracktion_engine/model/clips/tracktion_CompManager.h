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
*/
class CompManager   : public yup::SingleThreadedReferenceCountedObject,
                      private yup::ValueTree::Listener
{
public:
    /** Creates a CompManager for a given clip. */
    CompManager (Clip&, const yup::ValueTree&);

    /** Destructor. */
    ~CompManager() override;

    using Ptr = yup::ReferenceCountedObjectPtr<CompManager>;

    virtual void initialise();

    //==============================================================================
    virtual HashCode getBaseTakeHash (int takeIndex) const = 0;
    virtual double getTakeLength (int takeIndex) const = 0;
    virtual double getOffset() const = 0;
    virtual double getLoopLength() const = 0;
    virtual bool getAutoTempo() = 0;
    virtual double getSourceTempo() = 0;
    virtual yup::String getWarning()                   { return {}; }
    virtual float getRenderProgress() const             { return 1.0f; }
    virtual void discardCachedData()                    {}

    /** Triggers the render of the comp. Call this when you change the comp in some way. */
    virtual void triggerCompRender() = 0;

    /** Should flatten the comp and remove all other takes. */
    virtual void flattenTake (int takeIndex, bool deleteSourceFiles) = 0;

    /** Pastes an existing comp to this manager and returns the newly added tree. */
    virtual yup::ValueTree pasteComp (const yup::ValueTree& /*compTree*/) { return {}; }

    //==============================================================================
    Clip& getClip() const noexcept                      { return clip; }

    /** Sets a component to be updated during render processes. */
    virtual void setStripToUpdate (yup::Component*)    {}

    /** Returns true if the source should display a warning about using multi-tempo changes. */
    bool shouldDisplayWarning() const noexcept          { return displayWarning; }

    //==============================================================================
    /** Returns the section at the given index of a given take. */
    yup::ValueTree getSection (int takeIndex, int sectionIndex) const;

    /** Returns either the section for the current comp at a given time or if a whole
        take is being used the take. Check the type to find out which this is.
    */
    yup::ValueTree findSectionAtTime (double time);

    /** Returns the index of the section whose end lies within the given time range.
        This is useful when finding out what edges to drag etc. If no section is found
        then -1 is returned.
    */
    int findSectionWithEndTime (yup::Range<double> range, int takeIndex, bool& timeFoundAtStartOfSection) const;

    /** Returns the time range a given section occupies for a given take.
        If there is no segment at the indexes this will return an empty range.
    */
    yup::Range<double> getSectionTimes (const yup::ValueTree&) const;

    //==============================================================================
    yup::ValueTree getTakesTree()                      { return takesTree; }

    /** Sets the active take index. This will also update the source. */
    void setActiveTakeIndex (int index);

    /** Returns the active take index. */
    int getActiveTakeIndex() const                      { return clip.getCurrentTake(); }

    /** Returns the active take tree. */
    yup::ValueTree getActiveTakeTree() const           { return takesTree.getChild (getActiveTakeIndex()); }

    /** Returns the number of takes that are not comps. */
    int getNumTakes() const;

    /** Returns the number of comps that are comps. */
    int getNumComps() const                             { return takesTree.getNumChildren() - getNumTakes(); }

    /** Returns the total number of takes including comp takes. */
    int getTotalNumTakes() const                        { return takesTree.getNumChildren(); }

    /** Returns true if the given take at an index is a comp. */
    bool isTakeComp (int takeIndex) const               { return isTakeComp (takesTree.getChild (takeIndex)); }

    /** Returns true if the given take is a comp. */
    bool isTakeComp (const yup::ValueTree& takeTree) const   { return bool (takeTree.getProperty (IDs::isComp, false)); }

    /** Returns true if the current take is a comp. */
    bool isCurrentTakeComp() const                      { return isTakeComp (getActiveTakeIndex()); }

    /** Returns the name of a take. */
    yup::String getTakeName (int index) const;

    //==============================================================================
    /** Adds a new comp to the end of the takes list optionally making it active.
        This is essentially a new take with a new ProjectItemID and a single blank section.
    */
    virtual yup::ValueTree addNewComp() = 0;

    /** Returns the maximum length that a comp can be. This is effectively the length of the longest take. */
    double getMaxCompLength() const                 { return maxCompLength; }

    /** Returns the time range available for comping i.e. taking into account any offset or looped regions. */
    yup::Range<double> getCompRange() const;

    /** Returns the effective speed ratio used for displaying waveforms.
        If the source is using auto-tempo with tempo changes this will use an averaged version of this.
    */
    double getSpeedRatio() const;

    /** Returns the current time multiplier in use by the source, either the speed ratio or auto tempo ratio. */
    double getSourceTimeMultiplier() const          { return effectiveTimeMultiplier; }

    /** Returns a hash code representing a take. This can be used to check if a comp has changed
        since it was last generated.
    */
    HashCode getTakeHash (int takeIndex) const;

    //==============================================================================
    /** Changes the index of the active comp's section at a given time.
        If the active comp is just a normal take this will change the active take.
    */
    void changeSectionIndexAtTime (double time, int takeIndex);

    /** Removes a section from the comp at the given time if the section is at the given take index. */
    void removeSectionIndexAtTime (double time, int takeIndex);

    /** Moves a section's end time to the new time specified. If this crosses the boundry
        of another section the two sections will be merged.
        @returns    the index of the section that was being dragged
    */
    void moveSectionEndTime (yup::ValueTree& section, double newTime);

    /** Moves a section by the specified time delta also moving the previous section's end time
        by the same ammount. If this covers up another section the covered section will be removed.
        If this drags a section at either end either a new blank section will be created or the drag will be
        limited by the take's bounds.
        If any sections are removed due to an overlap this will return the new section index that this represents.
    */
    void moveSection (yup::ValueTree& section, double timeDelta);

    /** Moves a section to an absolute end time also moving the previous section's end time
        by the same ammount. If this covers up another section the covered section will be removed.
        If this drags a section at either end either a new blank section will be created or the drag will be
        limited by the take's bounds.
        If any sections are removed due to an overlap this will return the new section index that this represents.
    */
    void moveSectionToEndAt (yup::ValueTree& section, double newEndTime);

    /** Adds a new section at a given time and returns the index of it. */
    yup::ValueTree addSection (int takeIndex, double endTime);

    /** Removes a section from the active comp if it is within range. */
    void removeSection (const yup::ValueTree& sectionToRemove);

    /** Find the current section at the given time and splits it in two ready for a new comp section. */
    yup::ValueTree splitSectionAtTime (double time);

    /** Removes all sections which lie within the given time range. Useful when performing drag operations.
        @returns the number of sections removed removed.
    */
    void removeSectionsWithinRange (yup::Range<double> timeRange, const yup::ValueTree& sectionToKeep);

protected:
    yup::ValueTree takesTree;
    int lastRenderedTake = -1;
    HashCode lastHash = 0;
    double maxCompLength, effectiveTimeMultiplier;
    double lastOffset = 1.0, lastTimeRatio = 1.0;

    juce::UndoManager* getUndoManager() const;
    void keepSectionsSortedAndInRange();
    yup::ValueTree getNewCompTree() const;

private:
    //==============================================================================
    Clip& clip;
    bool displayWarning = false;

    //==============================================================================
    void reCacheInfo();
    void refreshCachedTakeLengths();
    void updateOffsetAndRatioFromSource();
    void addOrRemoveListenerIfNeeded();

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
    void valueTreeChildAdded (yup::ValueTree& p, yup::ValueTree&) override        { if (p == takesTree) reCacheInfo(); }
    void valueTreeChildRemoved (yup::ValueTree& p, yup::ValueTree&, int) override { if (p == takesTree) reCacheInfo(); }
    void valueTreeChildOrderChanged (yup::ValueTree&, int, int) override           {}
    void valueTreeParentChanged (yup::ValueTree&) override                         {}
    void valueTreeRedirected (yup::ValueTree&) override                            { jassertfalse; }

    //==============================================================================
    struct RenderTrigger;
    std::unique_ptr<RenderTrigger> renderTrigger;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompManager)
};

//==============================================================================
/**
*/
class CompFactory
{
public:
    CompFactory() = default;
    ~CompFactory() { jassert (comps.isEmpty()); }

    CompManager::Ptr getCompManager (Clip&);

private:
    friend class CompManager;
    yup::Array<CompManager*> comps;
    yup::CriticalSection compLock;

    void addComp (CompManager&);
    void removeComp (CompManager&);

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompFactory)
};

//==============================================================================
/**
*/
class WaveCompManager   : public CompManager,
                          private yup::Timer
{
public:
    using Ptr = yup::ReferenceCountedObjectPtr<WaveCompManager>;

    WaveCompManager (WaveAudioClip&);
    ~WaveCompManager() override;

    /** Updates an array of thumbnails so they represent the takes and are in the correct order etc. */
    void updateThumbnails (yup::Component&, yup::OwnedArray<SmartThumbnail>&) const;

    /** Returns the current comp file. */
    yup::File getCurrentCompFile() const;

    //==============================================================================
    HashCode getBaseTakeHash (int takeIndex) const override    { return getProjectItemRefForTake (takeIndex).toString().hashCode64(); }
    double getTakeLength (int takeIndex) const override;
    double getOffset() const override;
    double getLoopLength() const override;
    bool getAutoTempo() override;
    double getSourceTempo() override;
    yup::String getWarning() override;
    float getRenderProgress() const override;

    void triggerCompRender() override;
    void flattenTake (int takeIndex, bool deleteSourceFiles) override;
    yup::ValueTree pasteComp (const yup::ValueTree& compTree) override;
    void setStripToUpdate (yup::Component* strip) override;

    yup::ValueTree addNewComp() override;

    //==============================================================================
    struct CompRenderContext;

    /** Returns a context to render the current taste of this comp. */
    CompRenderContext* createRenderContext() const;

    /** Renders the comp using the given writer and ThreadPoolJob.
        This will return true if the comp was successfully completed or false if it failed.
        Note that this should only be called for comp takes and will simply return true for full takes.
    */
    static bool renderTake (CompRenderContext&, Edit&, AudioFileWriter&,
                            yup::ThreadPoolJob&, std::atomic<float>& progress);

private:
    enum { compGeneratorDelay = 500 };

    WaveAudioClip& clip;
    AudioFile lastCompFile;
    yup::String warning;

    //==============================================================================
    struct FlattenRetrier;
    std::unique_ptr<FlattenRetrier> flattenRetrier;

    struct CompUpdater;
    std::unique_ptr<CompUpdater> compUpdater;

    void setProjectItemIDForTake (int takeIndex, ProjectItemRef) const;
    void setSourceFileForTake (int takeIndex, const yup::File&) const;
    ProjectItemRef getProjectItemRefForTake (int takeIndex) const;
    AudioFile getSourceFileForTake (int takeIndex) const;
    yup::File getDefaultTakeFile (int index) const;
    ProjectItem::Ptr getOrCreateProjectItemForTake (yup::ValueTree& takeTree);

    void timerCallback() override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveCompManager)
};

//==============================================================================
/** */
class MidiCompManager   : public CompManager
{
public:
    using Ptr = yup::ReferenceCountedObjectPtr<MidiCompManager>;

    MidiCompManager (MidiClip&);
    ~MidiCompManager() override;

    void initialise() override;
    MidiList* getSequenceLooped (int index);

    //==============================================================================
    HashCode getBaseTakeHash (int takeIndex) const override;
    double getTakeLength (int takeIndex) const override;
    double getOffset() const override                       { return 0.0; }
    double getLoopLength() const override                   { return getTakeLength (0); }
    bool getAutoTempo() override                            { return false; }
    double getSourceTempo() override                        { return 1.0; }
    yup::String getWarning() override                      { return {}; }
    float getRenderProgress() const override                { return 1.0f; }
    void discardCachedData() override;

    void triggerCompRender() override;
    void flattenTake (int takeIndex, bool deleteSourceFiles) override;
    yup::ValueTree addNewComp() override;

private:
    MidiClip& clip;
    yup::ValueTree midiTakes;
    yup::OwnedArray<MidiList> cachedLoopSequences;

    void createComp (const yup::ValueTree& takeTree);

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiCompManager)
};

} // namespace tracktion::inline engine
