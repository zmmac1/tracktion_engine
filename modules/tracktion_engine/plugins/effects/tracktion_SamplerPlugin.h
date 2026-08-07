/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class SamplerPlugin  : public Plugin,
                       private yup::AsyncUpdater
{
public:
    SamplerPlugin (PluginCreationInfo);
    ~SamplerPlugin() override;

    //==============================================================================
    int getNumSounds() const;
    yup::String getSoundName (int index) const;
    void setSoundName (int index, const yup::String& name);
    AudioFile getSoundFile (int index) const;
    yup::String getSoundMedia (int index) const;
    int getKeyNote (int index) const;
    int getMinKey (int index) const;
    int getMaxKey (int index) const;
    float getSoundGainDb (int index) const;
    float getSoundPan (int index) const;
    bool isSoundOpenEnded (int index) const;
    double getSoundStartTime (int index) const;
    double getSoundLength (int index) const;
    void setSoundExcerpt (int index, double start, double length);

    // returns an error
    yup::String addSound (const yup::String& sourcePathOrProjectID, const yup::String& name,
                           double startTime, double length, float gainDb);
    void removeSound (int index);
    void setSoundParams (int index, int keyNote, int minNote, int maxNote);
    void setSoundGains (int index, float gainDb, float pan);
    void setSoundOpenEnded (int index, bool isOpenEnded);
    void setSoundMedia (int index, const yup::String& sourcePathOrProjectID);

    void playNotes (const yup::BigInteger& keysDown);
    void allNotesOff();

    /// If the state has changed and an update is pending, do it synchronously now
    void flushPendingUpdates();

    //==============================================================================
    static const char* getPluginName()                  { return NEEDS_TRANS("Sampler"); }
    static const char* xmlTypeName;

    yup::String getName() const override               { return TRANS("Sampler"); }
    yup::String getPluginType() override               { return xmlTypeName; }
    yup::String getShortName (int) override            { return "Smplr"; }
    yup::String getSelectableDescription() override    { return TRANS("Sampler"); }
    bool isSynth() override                             { return true; }

    int getNumOutputChannelsGivenInputs (int) override                 { return 2; }
    BusLayout getBusses() const override   { return BusLayout::singleStereoInOut(); }
    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;

    //==============================================================================
    bool takesMidiInput() override                      { return true; }
    bool takesAudioInput() override                     { return true; }
    bool producesAudioWhenNoAudioInput() override       { return true; }
    bool hasNameForMidiNoteNumber (int note, int midiChannel, yup::String& name) override;

    yup::Array<ReferencedItem> getReferencedItems() override;
    void reassignReferencedItem (const ReferencedItem&, ProjectItemRef newRef, double newStartTime) override;
    void sourceMediaChanged() override;

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    //==============================================================================
    struct SamplerSound
    {
        SamplerSound (SamplerPlugin&, const yup::String& sourcePathOrProjectID, const yup::String& name,
                      double startTime, double length, float gainDb);

        void setExcerpt (double startTime, double length);
        void refreshFile();

        SamplerPlugin& owner;
        yup::String source;
        yup::String name;
        int keyNote = -1, minNote = 0, maxNote = 0;
        int fileStartSample = 0, fileLengthSamples = 0;
        bool openEnded = false;
        float gainDb = 0, pan = 0;
        double startTime = 0, length = 0;
        AudioFile audioFile;
        yup::AudioBuffer<float> audioData { 2, 64 };

    private:
        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerSound)
    };

private:
    //==============================================================================
    struct SampledNote;

    yup::Color colour;
    yup::CriticalSection lock;
    yup::ReferenceCountedArray<SampledNote> playingNotes;
    yup::OwnedArray<SamplerSound> soundList;
    yup::BigInteger highlightedNotes;

    yup::ValueTree getSound (int index) const;

    void valueTreeChanged() override;
    void handleAsyncUpdate() override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerPlugin)
};

} // namespace tracktion::inline engine
