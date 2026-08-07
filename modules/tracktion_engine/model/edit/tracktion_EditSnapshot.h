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
    Holds a snapshot of an Edit file of the last time it was saved.
*/
class EditSnapshot  : public yup::ReferenceCountedObject
{
public:
    //==============================================================================
    using Ptr = yup::ReferenceCountedObjectPtr<EditSnapshot>;

    static Ptr getEditSnapshot (Engine&, yup::File);

    //==============================================================================
    struct Marker
    {
        yup::String name;
        yup::Color colour;
        TimeRange time;
    };

    //==============================================================================
    ~EditSnapshot();

    /** Returns the File if this was created from one. */
    yup::File getFile() const                          { return sourceFile; }

    /** Returns the source Xml. */
    yup::ValueTree getState() noexcept                 { return state; }

    /** Sets the Edit XML that the XmlEdit should refer to.
        This will take ownership of the XmlElement so don't hang on to it.
        Once this is set you can retrieve the Xml for saving etc. using getXml().
        As the length isn't stored in the Xml we need to provide it.

        Note that this DOES NOT update the cached properties, once the Xml has been set call
        refreshCacheAndSave to update these. Although this seems a bit convoluted it's how the
        temp Edit files work so we need to follow this. It means that this will hold up-to-date
        Xml but only cached properties of the last save which can be used in things like EditClips.
    */
    void setState (yup::ValueTree, TimeDuration editLength);

    /** Returns true if the current source is a valid Edit. */
    bool isValid() const;

    /** Refreshes the cached properties and calls any listeners. */
    void refreshCacheAndNotifyListeners();

    /** Full refresh reloading from the source file. */
    void refresh();

    /** deals only with snapshots so it's relatively fast. */
    yup::ReferenceCountedArray<EditSnapshot> getNestedEditObjects();

    //==============================================================================
    // Set of cached properties of the last saved state
    yup::String getName() const                                        { return name; }
    int getNumTracks() const                                            { return numTracks; }
    int getNumAudioTracks() const                                       { return numAudioTracks; }
    int getIndexOfTrackID (EditItemID trackID) const                    { return trackIDs.indexOf (trackID); }
    yup::String getTrackName (int index) const                         { return trackNames[index]; }
    yup::String getTrackNameFromID (EditItemID trackID) const          { return trackNames[getIndexOfTrackID(trackID)]; }
    bool isAudioTrack (int trackIndex) const                            { return audioTracks[trackIndex]; }
    bool isTrackMuted (int trackIndex) const                            { return mutedTracks[trackIndex]; }
    bool isTrackSoloed (int trackIndex) const                           { return soloedTracks[trackIndex]; }
    bool isTrackSoloIsolated (int trackIndex) const                     { return soloIsolatedTracks[trackIndex]; }
    int audioToGlobalTrackIndex (int audioIndex) const;

    TimeDuration getLength() const                                       { return TimeDuration::fromSeconds (length); }
    TimePosition getMarkIn() const                                       { return TimePosition::fromSeconds (markIn); }
    TimePosition getMarkOut() const                                      { return TimePosition::fromSeconds (markOut); }
    bool areMarksActive() const                                          { return marksActive; }

    double getTempo() const                                             { return tempo; }
    int getTimeSigNumerator() const                                     { return timeSigNumerator; }
    int getTimeSigDenominator() const                                   { return timeSigDenominator; }
    int getPitch() const                                                { return pitch; }

    const yup::Array<Marker>& getMarkers() const                       { return markers; }
    const yup::Array<EditItemID>& getTracks() const                    { return trackIDs; }
    const yup::Array<yup::File>& getEditClips() const                 { return editClipRefs; }
    const yup::Array<yup::File>& getClipsSourceRefs() const           { return clipSourceRefs; }
    HashCode getHash() const                                            { return lastSaveTime.toMilliseconds(); }

    //==============================================================================
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void editChanged (EditSnapshot&) = 0;
    };

    void addListener (Listener* l)          { listeners.add (l); }
    void removeListener (Listener* l)       { listeners.remove (l); }

    Engine& engine;

private:
    //==============================================================================
    struct ListHolder;
    std::unique_ptr<ListHolder> listHolder;

    Project::Ptr project;
    yup::File sourceFile;
    yup::ValueTree state;
    yup::Time lastSaveTime;

    yup::String name;
    int numTracks, numAudioTracks;
    yup::StringArray trackNames;
    yup::BigInteger audioTracks, mutedTracks, soloedTracks, soloIsolatedTracks;

    yup::Array<EditItemID> trackIDs;
    yup::Array<yup::File> editClipRefs, clipSourceRefs;
    double length = 0.0, markIn = 0.0, markOut = 0.0, tempo = 0.0;
    bool marksActive = false;
    int timeSigNumerator = 4, timeSigDenominator = 4, pitch = 60;
    yup::Array<Marker> markers;

    yup::ListenerList<Listener> listeners;

    //==============================================================================
    EditSnapshot (Engine&, yup::File);
    void refreshFromXml (const yup::XmlElement&, const yup::String&, double newLength);
    void refreshFromState();
    void clear();
    void addEditClips (const yup::XmlElement& track);
    void addClipSources (const yup::XmlElement& track);
    void addMarkers (const yup::XmlElement& track);
    void addSubTracksRecursively (const yup::XmlElement& parent, int& audioTrackNameNumber);

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditSnapshot)
};

} // namespace tracktion::inline engine
