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
class MidiInputDevice : public InputDevice,
                        public yup::MidiInputCallback,
                        private yup::Timer,
                        private yup::MidiKeyboardStateListener
{
public:
    MidiInputDevice (Engine&, yup::String name, yup::String deviceID);
    ~MidiInputDevice() override;

    virtual yup::String openDevice() = 0;
    virtual void closeDevice() = 0;

    void setEnabled (bool) override;
    bool isMidi() const override                    { return true; }

    MidiChannel getChannelToUse() const noexcept    { return channelToUse; }
    void setChannelToUse (int);

    int getProgramToUse() const noexcept            { return programToUse; }
    void setProgramToUse (int);

    void setBankToUse (int);
    int getBankToUse() const                        { return bankToUse; }

    void setChannelAllowed (int midiChannel, bool);
    bool isChannelAllowed (int midiChannel) const   { return ! disallowedChannels[midiChannel - 1]; }

    struct NoteFilterRange
    {
        int startNote = 0, endNote = 128;

        void set (int start, int end)
        {
            startNote = std::min (127, std::max (0, start));
            endNote = std::min (128, std::max (startNote + 1, end));
        }

        bool isAllNotes() const     { return startNote == 0 && endNote == 128; }
    };

    void setNoteFilterRange (NoteFilterRange);
    NoteFilterRange getNoteFilterRange() const      { return noteFilterRange; }

    void setOverridingNoteVelocities (bool);
    bool isOverridingNoteVelocities() const         { return overrideNoteVels; }

    void setManualAdjustmentMs (double);
    double getManualAdjustmentMs() const            { return manualAdjustMs; }

    void setMinimumLengthMs (double);
    double getMinimumLengthMs() const               { return minimumLengthMs; }

    /** Returns true if the given device is an MPE device and so should always record incoming MIDI to Note Expression. */
    bool isMPEDevice() const;

    //==============================================================================
    void masterTimeUpdate (double time) override;
    void connectionStateChanged();

    /// Updates the timestamp of the message and handles sending it out to
    /// listeners.  false if the given message has been filtered out by channel
    /// or  number.
    bool handleIncomingMessage (yup::MidiMessage&);

    //==============================================================================
    void addInstance (MidiInputDeviceInstanceBase*);
    void removeInstance (MidiInputDeviceInstanceBase*);

    virtual void loadProps() = 0;

    bool mergeRecordings = true;
    bool recordingEnabled = true;
    bool replaceExistingClips = false;
    bool recordToNoteAutomation = false;
    QuantisationType quantisation;

    enum class MergeMode { always, never, optional };

    Clip* addMidiAsTransaction (Edit&, EditItemID targetID,
                                Clip* takeClip, yup::MidiMessageSequence,
                                TimeRange markedRange, MergeMode, MidiChannel);

    yup::MidiKeyboardState keyboardState;

    void handleIncomingMidiMessage (yup::MidiInput*, const yup::MidiMessage&) override;
    virtual void handleIncomingMidiMessage (const yup::MidiMessage&, MPESourceID) = 0;

    RetrospectiveMidiBuffer* getRetrospectiveMidiBuffer() const     { return retrospectiveBuffer.get(); }
    void updateRetrospectiveBufferLength (double length) override;
    double getAdjustSecs() const                                    { return adjustSecs; }

    yup::Array<AudioTrack*> getDestinationTracks();

    MidiChannel getMidiChannelFor (int rawChannelNumber) const;
    MPESourceID getMPESourceID() const                              { return midiSourceID; }

    //==============================================================================
    /** Gets notified (lazily, not in real-time) when any MidiInputDevice's key's state changes. */
    struct MidiKeyChangeDispatcher
    {
        struct Listener
        {
            virtual ~Listener() {}

            /** Callback to indicate notes have been played for the given track. */
            virtual void midiKeyStateChanged (AudioTrack*, const yup::Array<int>& notesOn, const yup::Array<int>& vels, const yup::Array<int>& notesOff) = 0;
        };

        yup::ListenerList<Listener> listeners;

    private:
        friend class yup::SharedResourcePointer<MidiKeyChangeDispatcher>;
        MidiKeyChangeDispatcher() {}
    };

protected:
    class MidiEventSnifferNode;
    class NoteDispatcher;

    std::atomic<double> adjustSecs { 0 };
    double manualAdjustMs = 0;
    double minimumLengthMs = 0;
    bool overrideNoteVels = false, eventReceivedFromDevice = false;
    yup::BigInteger disallowedChannels;
    MidiChannel channelToUse;
    int programToUse = 0;
    int bankToUse = 0;
    NoteFilterRange noteFilterRange;

    MPESourceID midiSourceID = createUniqueMPESourceID();

    std::unique_ptr<NoteDispatcher> noteDispatcher;
    std::vector<double> lastNoteOns;

    yup::CriticalSection noteLock;
    bool keysDown[128], keysUp[128];
    uint8_t keyDownVelocities[128];
    yup::SharedResourcePointer<MidiKeyChangeDispatcher> midiKeyChangeDispatcher;

    yup::CriticalSection instanceLock;
    yup::Array<MidiInputDeviceInstanceBase*> instances;
    std::unique_ptr<RetrospectiveMidiBuffer> retrospectiveBuffer;

    void handleNoteOn (yup::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (yup::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    void timerCallback() override;

    void sendNoteOnToMidiKeyListeners (yup::MidiMessage&);

    void loadMidiProps (const yup::XmlElement*);
    void saveMidiProps (yup::XmlElement&);

    void sendMessageToInstances (const yup::MidiMessage&, MPESourceID);

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInputDevice)
};

} // namespace tracktion::inline engine
