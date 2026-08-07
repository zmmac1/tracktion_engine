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
class StepClip   : public Clip,
                   public yup::ChangeBroadcaster
{
public:
    StepClip (const yup::ValueTree&, EditItemID, ClipOwner&);
    ~StepClip() override;

    using Ptr = yup::ReferenceCountedObjectPtr<StepClip>;

    void cloneFrom (Clip*) override;

    //==============================================================================
    struct Channel  : public Selectable
    {
        Channel (StepClip&, const yup::ValueTree&);
        ~Channel() noexcept override;

        bool operator== (const Channel&) const noexcept;

        yup::String getSelectableDescription() override;

        int getIndex() const;
        yup::String getDisplayName() const;

        bool usesGrooveStrength() const;

        StepClip& clip;
        yup::ValueTree state;

        juce::CachedValue<MidiChannel> channel;  // MIDI channel. Range: 1 - 16
        juce::CachedValue<int> noteNumber;       // Range: 0 - 127
        juce::CachedValue<int> noteValue;        // Most likely to be ranged from 0 to 127
        juce::CachedValue<yup::String> grooveTemplate, name;
        juce::CachedValue<float> grooveStrength;

    private:
        YUP_LEAK_DETECTOR (Channel)
    };

    const yup::Array<Channel*>& getChannels() const noexcept;
    void removeChannel (int index);
    void insertNewChannel (int index);

    bool usesProbability();

    //==============================================================================
    /**
    */
    struct Pattern
    {
        Pattern (StepClip&, const yup::ValueTree&) noexcept;
        Pattern (const Pattern&) noexcept;

        yup::String getName() const;
        void setName (const yup::String&);

        bool getNote (int channel, int index) const noexcept;
        void setNote (int channel, int index, bool value);

        void clear();
        void clearChannel (int channel);
        void insertChannel (int channel);
        void removeChannel (int channel);
        void randomiseChannel (int channel);
        void randomiseSteps();
        void shiftChannel (int channel, bool toTheRight);
        void toggleAtInterval (int channel, int interval);

        yup::BigInteger getChannel (int index) const;
        void setChannel (int index, const yup::BigInteger&);

        int getNumNotes() const;
        void setNumNotes (int);

        /** Returns the length of one step as a fraction of a beat. */
        BeatDuration getNoteLength() const;

        /** Sets the length of one step as a fraction of a beat. */
        void setNoteLength (BeatDuration);

        yup::Array<int> getVelocities (int channel) const;
        void setVelocities (int channel, const yup::Array<int>&);

        int getVelocity (int channel, int index) const;
        void setVelocity (int channel, int index, int value);

        yup::Array<double> getGates (int channel) const;
        void setGates (int channel, const yup::Array<double>&);

        double getGate (int channel, int index) const;
        void setGate (int channel, int index, double value);

        yup::Array<float> getProbabilities (int channel) const;
        void setProbabilities (int channel, const yup::Array<float>&);

        float getProbability (int channel, int index) const;
        void setProbability (int channel, int index, float value);

        /** Creates a snapshot of a pattern's notes, velocities and gates to avoid costly
            property parsing. Obviously if you change a property this will become invalid.
         */
        struct CachedPattern
        {
            CachedPattern (const Pattern&, int channel);

            bool getNote (int index) const noexcept;
            int getVelocity (int index) const noexcept;
            double getGate (int index) const noexcept;
            float getProbability (int index) const noexcept;

            yup::BigInteger notes;
            yup::Array<int> velocities;
            yup::Array<double> gates;
            yup::Array<float> probabilities;
            YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedPattern)
        };

        StepClip& clip;
        yup::ValueTree state;

    private:
        Pattern& operator= (const Pattern&) = delete;
        YUP_LEAK_DETECTOR (Pattern)
    };

    //==============================================================================
    /**
    */
    struct PatternInstance  : public Selectable,
                              public yup::ReferenceCountedObject
    {
        PatternInstance (StepClip& c, int index)
            : clip (c), patternIndex (index) {}

        ~PatternInstance() override
        {
            notifyListenersOfDeletion();
        }

        using Ptr = yup::ReferenceCountedObjectPtr<PatternInstance>;

        StepClip& clip;
        int patternIndex;

        Pattern getPattern() const;
        int getSequenceIndex() const;

        yup::String getSelectableDescription() override;

    private:
        PatternInstance() = delete;
        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PatternInstance)
    };

    //==============================================================================
    enum Defaults
    {
        defaultNumNotes     = 16,
        defaultNumChannels  = 8,
        defaultMidiChannel  = 1,
        defaultNoteNumber   = 60,
        defaultNoteValue    = 96,

        minNumNotes         = 2,
        maxNumNotes         = 512,

        minNumChannels      = 1,
        maxNumChannels      = 60
    };

    //==============================================================================
    using PatternArray = yup::ReferenceCountedArray<PatternInstance>;

    PatternArray getPatternSequence() const;
    void setPatternSequence (const PatternArray&);

    void insertVariation (int patternIndex, int insertIndex);
    void removeVariation (int variationIndex);
    void removeAllVariations();
    void createDefaultPatternIfEmpty();
    void setPatternForVariation (int variationIndex, int newPatternIndex);

    //==============================================================================
    int insertPattern (const Pattern&, int index);
    int insertNewPattern (int index);
    void removePattern (int patternIndex);
    void removeUnusedPatterns();

    //==============================================================================
    bool getCell (int patternIndex, int channelIndex, int noteIndex);
    void setCell (int patternIndex, int channelIndex, int noteIndex, bool value);

    //==============================================================================
    Pattern getPattern (int index);
    int getNumPatterns() const;
    yup::Array<Pattern> getPatterns();

    //==============================================================================
    float getVolumeDb() const                       { return level->dbGain.get(); }
    void setVolumeDb (float v)                      { level->dbGain = yup::jlimit (-100.0f, 0.0f, v); }

    //==============================================================================
    /** Generate a MidiMessageSequence from either the entire clip or
        one of its pattern instances.

        @param[out] result          Result sequence to add the messages to
        @param[in] convertToSeconds Leave the timestamps in beats, or convert them to seconds while generating
        @param[in] instance         Specific pattern to generate, nullptr for the whole clip
    */
    void generateMidiSequence (yup::MidiMessageSequence& result,
                               bool convertToSeconds = true,
                               PatternInstance* instance = nullptr);

    /** Generate a MidiMessageSequence from either the entire clip or
        one of its pattern instances.

        @param[in] PatternInstance  Specific pattern to generate, nullptr for the whole clip
        @param[in] TimeBase         The time base for the sequence
        @returns result             Result sequence
    */
    yup::MidiMessageSequence generateMidiSequence (MidiList::TimeBase, PatternInstance* instance = nullptr);

    yup::Array<BeatPosition> getBeatTimesOfPatternStarts() const;

    BeatPosition getStartBeatOf (PatternInstance*);
    BeatPosition getEndBeatOf (PatternInstance*);

    int getBeatsPerBar();

    void resizeClipForPatternInstances();

    //==============================================================================
    bool canBeAddedTo (ClipOwner&) override;
    yup::String getSelectableDescription() override;
    yup::Color getDefaultColor() const override;

    bool isMidi() const override                        { return false; }
    bool beatBasedLooping() const override              { return true; }
    bool isMuted() const override                       { return level->mute.get(); }
    void setMuted (bool m) override                     { level->mute = m; }

    LiveClipLevel getLiveClipLevel();

    juce::CachedValue<bool> repeatSequence;

    //==============================================================================
    /** StepClips can only loop if they're being used as launcher clips.
        Arranger clips will automatically loop until their end time.
    */
    bool canLoop() const override;

    /** @internal */
    ClipPosition getPosition() const override;
    /** @internal */
    bool isLooping() const override                 { return canLoop() && loopLengthBeats > 0_bd; }
    /** @internal */
    void setNumberOfLoops (int) override;
    /** @internal */
    void disableLooping() override;
    /** @internal */
    void setLoopRange (TimeRange) override;
    /** @internal */
    void setLoopRangeBeats (BeatRange) override;
    /** @internal */
    BeatPosition getLoopStartBeats() const override       { return canLoop() ? loopStartBeats : 0_bp; }
    /** @internal */
    BeatDuration getLoopLengthBeats() const override      { return canLoop() ? loopLengthBeats : 0_bd; }
    /** @internal */
    TimePosition getLoopStart() const override;
    /** @internal */
    TimeDuration getLoopLength() const override;

    /** @internal */
    std::shared_ptr<LaunchHandle> getLaunchHandle() override;
    /** @internal */
    void setUsesGlobalLaunchQuatisation (bool useGlobal) override           { useClipLaunchQuantisation = ! useGlobal; }
    /** @internal */
    bool usesGlobalLaunchQuatisation() override                             { return ! useClipLaunchQuantisation; }
    /** @internal */
    LaunchQuantisation* getLaunchQuantisation() override;
    /** @internal */
    FollowActions* getFollowActions() override;

private:
    void generateMidiSequenceForChannels (yup::MidiMessageSequence&, bool convertToSeconds,
                                          const Pattern&, BeatPosition startBeat,
                                          BeatPosition clipStartBeat, BeatPosition clipEndBeat, const TempoSequence&);

    //==============================================================================
    struct ChannelList;
    std::unique_ptr<ChannelList> channelList;
    PatternArray patternInstanceList;
    std::shared_ptr<ClipLevel> level { std::make_shared<ClipLevel>() };
    juce::CachedValue<BeatPosition> loopStartBeats;
    juce::CachedValue<BeatDuration> loopLengthBeats, originalLength;
    std::shared_ptr<LaunchHandle> launchHandle;
    juce::CachedValue<bool> useClipLaunchQuantisation;
    std::unique_ptr<LaunchQuantisation> launchQuantisation;
    std::unique_ptr<FollowActions> followActions;

    const PatternInstance::Ptr getPatternInstance (int index, bool repeatSequence) const;
    void updatePatternList();

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override;
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override;
    void valueTreeChildOrderChanged (yup::ValueTree&, int, int) override;

    //==============================================================================
    StepClip() = delete;
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepClip)
};

} // namespace tracktion::inline engine
