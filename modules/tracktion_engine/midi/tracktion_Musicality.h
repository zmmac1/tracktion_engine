/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class Chord
{
public:
    enum ChordType
    {
        customChord = -2,
        invalidChord = -1,
        majorTriad = 0,
        minorTriad,
        diminishedTriad,
        augmentedTriad,
        majorSixthChord,
        minorSixthChord,
        dominatSeventhChord,
        majorSeventhChord,
        minorSeventhChord,
        augmentedSeventhChord,
        diminishedSeventhChord,
        halfDiminishedSeventhChord,
        minorMajorSeventhChord,
        suspendedSecond,
        suspendedFourth,
        powerChord,
        majorNinthChord,
        dominantNinthChord,
        minorMajorNinthChord,
        minorDominantNinthChord,
        augmentedMajorNinthChord,
        augmentedDominantNinthChord,
        halfDiminishedNinthChord,
        halfDiminishedMinorNinthChord,
        diminishedNinthChord,
        diminishedMinorNinthChord,
        // Update getAllChordType after adding / removing a chord
    };

    Chord (ChordType type = majorTriad);
    Chord (yup::Array<int> steps, yup::String symbol); // creates a custom chord with steps

    yup::String toString();
    static Chord fromString (const yup::String&);

    static yup::Array<ChordType> getAllChordType();

    ChordType getType() const   { return type; }
    bool isValid() const        { return type != invalidChord; }

    yup::String getName() const;
    yup::String getShortName() const;
    yup::String getSymbol() const;
    yup::Array<int> getSteps() const;
    yup::Array<int> getSteps (int inversion) const;

private:
    ChordType type;
    yup::Array<int> steps; // for custom chords
    yup::String symbol;    // for custom chords
};

//==============================================================================
class Scale
{
public:
    enum ScaleType
    {
        major = 0,
        minor,
        ionian,
        dorian,
        phrygian,
        lydian,
        mixolydian,
        aeolian,
        locrian,
        melodicMinor,
        harmonicMinor,
    };

    enum Steps
    {
        Whole = 0,
        Half,
        WholeHalf,
    };

    enum class Intervals
    {
        i = 0,
        ii,
        iii,
        iv,
        v,
        vi,
        vii
    };

    Scale (ScaleType type = major);

    ScaleType getType() const { return type; }

    yup::String getName() const;
    yup::String getShortName() const;

    static yup::StringArray getIntervalNames();

    static yup::Array<ScaleType> getAllScaleTypes();
    static yup::StringArray getScaleStrings();
    static yup::String getNameForType (ScaleType type);
    static yup::String getShortNameForType (ScaleType type);
    static ScaleType getTypeFromName (yup::String name);

    yup::Array<int> getSteps (int octaves = 1) const;

    yup::String getIntervalName (Intervals interval) const;

    yup::Array<Chord> getTriads()   const { return triads;   }
    yup::Array<Chord> getSixths()   const { return sixths;   }
    yup::Array<Chord> getSevenths() const { return sevenths; }
    yup::Array<Chord> getNinths()   const { return ninths;   }

private:
    yup::Array<Chord> generateTriads (int offset) const;
    yup::Array<Chord> generateSixths (int offset) const;
    yup::Array<Chord> generateSevenths (int offset) const;
    yup::Array<Chord> generateNinths (int offset) const;

    ScaleType type;
    yup::Array<Steps> steps;
    yup::Array<Chord> triads;
    yup::Array<Chord> sixths;
    yup::Array<Chord> sevenths;
    yup::Array<Chord> ninths;
};

//==============================================================================
class PatternGenerator  : private ValueTreeAllEventListener
{
public:
    enum class Mode
    {
        off,
        arpeggio,
        chords,
        bass,
        melody
    };

    static const int scaleRootGlobalTrack;
    static const int scaleRootChordTrack;

    //==============================================================================
    PatternGenerator (Clip&, yup::ValueTree);
    ~PatternGenerator() override;

    //==============================================================================
    BeatDuration getMinimumChordLength() const;
    BeatDuration getMaximumChordLength() const;

    void validateChordLengths();

    //==============================================================================
    struct ProgressionItem
    {
        ProgressionItem (PatternGenerator&, const yup::ValueTree&, bool temporary = false);
        ~ProgressionItem() noexcept;

        bool operator== (const ProgressionItem&) const noexcept;
        bool isValid() { return chordName.get().isNotEmpty(); }

        PatternGenerator& generator;
        yup::ValueTree state;

        juce::CachedValue<yup::String> chordName;  // Chord name is simplified, use getChordName() to get unsimplified
        juce::CachedValue<yup::String> pitches;    // A comma seperated list of pitches used for custom chords, otherwise empty
        juce::CachedValue<BeatDuration> lengthInBeats;
        juce::CachedValue<int> octave, inversion;

        void setChordName (yup::String chord);
        void setChordName (yup::String chord, yup::String pitches);

        yup::String getChordName() const;

        void setRoot (int root);
        void setChord (int root, Chord::ChordType);

        bool isRomanNumeral() const;                       // Is this chord stored in it's roman numberal format

        Chord getChord (const Scale&) const;
        yup::String getChordSymbol();              // Gets the chords name as Cmaj, Dmin, etc based on key

        int getRootNote (int key, const Scale& scale);

    private:
        YUP_LEAK_DETECTOR (ProgressionItem)
    };

    //==============================================================================

    yup::StringArray getPossibleTriadNames() const;
    yup::StringArray getPossibleSixthNames() const;
    yup::StringArray getPossibleSeventhNames() const;
    yup::StringArray getPossibleNinthNames() const;

    int getChordProgressionLength() const;
    const yup::Array<ProgressionItem*>& getChordProgression() const noexcept;
    void setChordProgression (yup::ValueTree v);

    yup::StringArray getChordProgressionChordNames (bool simplified) const;

    /** Sets a chord progression using chord roman numerals.
        Be careful, this sets everything else to default. */
    void setChordProgressionFromChordNames (yup::StringArray progression);

    void removeIndexFromProgression (int idx);
    void removeRangeFromProgression (int start, int end);
    void clearProgression();
    void insertChordIntoProgression (int idx, yup::String chordName);
    void insertChordIntoProgression (int idx, yup::String chordName, yup::String pitches);
    void moveChordInProgression (int srcIdx, int dstIdx);
    void duplicateChordInProgression (int idx);

    void playGuideChord (int idx) const;

    //==============================================================================
    enum NoteType
    {
        ChordInKeyNote,
        ChordNotInKeyNote,
        InKeyNote,
        NotInKeyNote
    };

    NoteType getTypeForNote (const MidiClip&, const MidiNote&);

    //==============================================================================
    yup::String formatChordName (yup::String simplifiedChordName) const;
    yup::StringArray getArpStyles();

    Clip& clip;
    yup::ValueTree state;
    juce::CachedValue<Mode> mode;
    juce::CachedValue<Scale::ScaleType> scaleType;
    juce::CachedValue<bool> autoUpdate, arpUpDown, arpPlayRoot, allNotes, octaveUp, octaveDown, spread;
    juce::CachedValue<float> arpPatternLength, velocity, gate;
    juce::CachedValue<BeatDuration> melodyNoteLength;
    juce::CachedValue<yup::String> arpStyle;
    juce::CachedValue<int> scaleRoot, arpSteps, octave;
    juce::CachedValue<yup::int64> patternHash;

    yup::ValueTree getChordPattern();
    yup::ValueTree getBassPattern();

    void setChordPattern (yup::ValueTree pattern);
    void setBassPattern (yup::ValueTree pattern);

    void generatePattern();

    Scale getScaleAtBeat (BeatPosition) const;
    int getNoteAtBeat (BeatPosition) const;

    bool getAutoUpdate();
    void setAutoUpdate (bool on);
    void refreshPatternIfNeeded();

    void editFinishedLoading();

    BeatDuration getFlattenedChordProgression (yup::OwnedArray<ProgressionItem>& progression, bool globalTime = false);

private:
    const int maxChords = 64;

    struct ProgressionList;
    std::unique_ptr<ProgressionList> progressionList;

    void valueTreeChanged() override {}
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;

    ChordClip* getChordClipAt (TimePosition) const;

    MidiNote* addNote (MidiList& sequence, int pitch, BeatPosition startBeat, BeatDuration lengthInBeats,
                       int velocity, int colourIndex, juce::UndoManager*);

    void clearPattern();
    void generateArpPattern();
    void generateChordPattern();
    void generateBassPattern();
    void generateMelodyPattern();

    void updateHash();
    void clearHash();
    HashCode hashNotes (MidiList&, int version);

    MidiClip* getMidiClip() const;

    //==============================================================================
    std::unique_ptr<yup::Timer> editLoadedCallback;

    //==============================================================================
    struct AutoUpdateManager;
    std::unique_ptr<AutoUpdateManager> autoUpdateManager;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PatternGenerator)
};

//==============================================================================
struct KeyResult
{
    KeyResult() = default;
    KeyResult (double r_, int k_, Scale::ScaleType s_) : r (r_), key (k_), scale (s_) {}

    double r = 0; // correlation coefficient
    int key = 0;  // midi note = 0 - 11
    Scale::ScaleType scale = Scale::major; // major or minor

    bool operator< (const KeyResult& other) const { return r < other.r; }
};

yup::Array<KeyResult> determineKeyOfNotes (const yup::Array<MidiNote*>& notes);

} // namespace tracktion::inline engine

namespace yup
{
    template <>
    struct VariantConverter<tracktion::engine::Chord::ChordType>
    {
        static tracktion::engine::Chord::ChordType fromVar (const var& v)   { return (tracktion::engine::Chord::ChordType) static_cast<int> (v); }
        static var toVar (tracktion::engine::Chord::ChordType v)            { return static_cast<int> (v); }
    };


    template <>
    struct VariantConverter<tracktion::engine::Scale::ScaleType>
    {
        static tracktion::engine::Scale::ScaleType fromVar (const var& v)   { return (tracktion::engine::Scale::ScaleType) static_cast<int> (v); }
        static var toVar (tracktion::engine::Scale::ScaleType v)            { return static_cast<int> (v); }
    };

    template <>
    struct VariantConverter<tracktion::engine::PatternGenerator::Mode>
    {
        static tracktion::engine::PatternGenerator::Mode fromVar (const var& v)   { return (tracktion::engine::PatternGenerator::Mode) static_cast<int> (v); }
        static var toVar (tracktion::engine::PatternGenerator::Mode v)            { return static_cast<int> (v); }
    };
}
