/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class MidiSysexEvent
{
public:
    static yup::ValueTree createSysexEvent (const MidiSysexEvent&, BeatPosition);
    static yup::ValueTree createSysexEvent (const yup::MidiMessage&, BeatPosition);

    MidiSysexEvent (const yup::ValueTree&);
    MidiSysexEvent (const yup::MidiMessage&);
    MidiSysexEvent (const yup::MidiMessage&, BeatPosition);
    ~MidiSysexEvent() noexcept {}

    //==============================================================================
    const yup::MidiMessage& getMessage() const noexcept            { return message; }
    void setMessage (const yup::MidiMessage&, juce::UndoManager*);

    //==============================================================================
    BeatPosition getBeatPosition() const noexcept                         { return BeatPosition::fromBeats (message.getTimeStamp()); }
    void setBeatPosition (BeatPosition, juce::UndoManager*);

    // takes into account quantising, groove templates, clip offset, etc
    TimePosition getEditTime (const MidiClip&) const;
    BeatPosition getEditBeats (const MidiClip&) const;

    yup::ValueTree state;

private:
    //==============================================================================
    friend class MidiList;
    yup::MidiMessage message;

    void updateMessage();
    void updateTime();

    MidiSysexEvent() = delete;
    MidiSysexEvent (const MidiSysexEvent&) = delete;

    YUP_LEAK_DETECTOR (MidiSysexEvent)
};

} // namespace tracktion::inline engine
