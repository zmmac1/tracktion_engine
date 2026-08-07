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
class TrackCompManager
{
public:
    TrackCompManager (Edit&);
    ~TrackCompManager();

    void initialise (const yup::ValueTree&);
    int getNumGroups() const;

    yup::StringArray getCompNames() const;
    yup::String getCompName (int index);
    void setCompName (int index, const yup::String& name);

    int addGroup (const yup::String& name);
    void removeGroup (int index);

    yup::Array<Track*> getTracksInComp (int index);

    //==============================================================================
    struct CompSection  : public yup::ReferenceCountedObject
    {
        CompSection (const yup::ValueTree&);
        ~CompSection();

        using Ptr = yup::ReferenceCountedObjectPtr<CompSection>;
        static CompSection* createAndIncRefCount (const yup::ValueTree& v);

        void updateFrom (yup::ValueTree&, const yup::Identifier&);

        yup::ValueTree state;

        EditItemID getTrack() const      { return track; }
        double getEnd() const      { return end; }

    private:
        EditItemID track;
        double end = 0.0;

        void updateTrack();
        void updateEnd();

        YUP_DECLARE_WEAK_REFERENCEABLE (CompSection)
        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompSection)
    };

    //==============================================================================
    struct TrackComp    : public Selectable,
                          public yup::ReferenceCountedObject,
                          public ValueTreeObjectList<CompSection>
    {
        enum TimeFormat
        {
            seconds = 0,
            beats
        };

        TrackComp (Edit&, const yup::ValueTree&);
        ~TrackComp() override;

        using Ptr = yup::ReferenceCountedObjectPtr<TrackComp>;
        static TrackComp* createAndIncRefCount (Edit&, const yup::ValueTree&);

        yup::String getSelectableDescription() override;

        TimeFormat getTimeFormat() const noexcept       { return timeFormat; }

        void setTimeFormat (TimeFormat);

        static yup::Array<TimeRange> getMuteTimes (const yup::Array<TimeRange>& nonMuteTimes);
        yup::Array<TimeRange> getNonMuteTimes (Track&, TimeDuration crossfadeTime) const;
        TimeRange getTimeRange() const;

        void setName (const yup::String&);

        static EditItemID getSourceTrackID (const yup::ValueTree&);
        static void setSourceTrackID (yup::ValueTree&, EditItemID, juce::UndoManager*);

        void setSectionTrack (CompSection&, const Track::Ptr&);
        void removeSection (CompSection&);

        CompSection* moveSectionToEndAt (CompSection*, double newEndTime);
        CompSection* moveSection (CompSection*, double timeDelta);
        CompSection* moveSectionEndTime (CompSection*, double newTime);

        int removeSectionsWithinRange (yup::Range<double>, CompSection* sectionToKeep);
        yup::ValueTree addSection (EditItemID trackID, double endTime, juce::UndoManager*);
        CompSection* addSection (Track::Ptr, double endTime);
        CompSection* splitSectionAtTime (double);

        CompSection* findSectionWithEdgeTimeWithin (const Track::Ptr&, yup::Range<double>, bool& startEdge) const;
        CompSection* findSectionAtTime (const Track::Ptr&, double) const;

        struct Section
        {
            CompSection* compSection;
            yup::Range<double> timeRange;
        };

        yup::Array<Section> getSectionsForTrack (const Track::Ptr&) const;
        bool isSuitableType (const yup::ValueTree&) const override;
        CompSection* createNewObject (const yup::ValueTree&) override;
        void deleteObject (CompSection*) override;
        void newObjectAdded (CompSection*) override;
        void objectRemoved (CompSection*) override;
        void objectOrderChanged() override;
        void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;

        yup::ValueTree state;
        Edit& edit;

        juce::CachedValue<yup::String> name;
        juce::CachedValue<yup::Color> includedColor, excludedColor;

    private:
        juce::CachedValue<TimeFormat> timeFormat;

        CompSection* getCompSectionFor (const yup::ValueTree&);
        void convertFromSecondsToBeats();
        void convertFromBeatsToSeconds();
        void convertTimes (TimeFormat, TimeFormat);

        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComp)
    };

    TrackComp::Ptr getTrackComp (AudioTrack*);

private:
    struct TrackCompList;

    Edit& edit;
    yup::ValueTree state;
    std::unique_ptr<TrackCompList> trackCompList;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackCompManager)
};

} // namespace tracktion::inline engine

namespace yup
{
    template <>
    struct VariantConverter<tracktion::engine::TrackCompManager::TrackComp::TimeFormat>
    {
        static tracktion::engine::TrackCompManager::TrackComp::TimeFormat fromVar (const var& v)   { return (tracktion::engine::TrackCompManager::TrackComp::TimeFormat) static_cast<int> (v); }
        static var toVar (tracktion::engine::TrackCompManager::TrackComp::TimeFormat v)            { return static_cast<int> (v); }
    };
}
