/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** Represents a set of selected MIDI notes.
    This will broadcast a change message when the events that are selected are changed.
*/
class SelectedMidiEvents   : public Selectable,
                             public yup::ChangeBroadcaster
{
public:
    SelectedMidiEvents (MidiClip&);
    SelectedMidiEvents (yup::Array<MidiClip*>);
    ~SelectedMidiEvents() override;

    Edit& getEdit();

    //==============================================================================
    MidiClip* clipForEvent (MidiNote*) const;
    MidiClip* clipForEvent (MidiSysexEvent*) const;
    MidiClip* clipForEvent (MidiControllerEvent*) const;

    //==============================================================================
    void addSelectedEvent (MidiNote*, bool addToCurrent);
    void addSelectedEvent (MidiSysexEvent*, bool addToCurrent);
    void addSelectedEvent (MidiControllerEvent*, bool addToCurrent);
    void removeSelectedEvent (MidiNote*);
    void removeSelectedEvent (MidiSysexEvent*);
    void removeSelectedEvent (MidiControllerEvent*);
    void setSelected (SelectionManager&, const yup::Array<MidiNote*>&, bool addToSelection, bool allowMixedSelection = false);
    void setSelected (SelectionManager&, const yup::Array<MidiSysexEvent*>&, bool addToSelection, bool allowMixedSelection = false);
    void setSelected (SelectionManager&, const yup::Array<MidiControllerEvent*>&, bool addToSelection, bool allowMixedSelection = false);
    bool isSelected (const MidiNote*) const;
    bool isSelected (const MidiSysexEvent*) const;
    bool isSelected (const MidiControllerEvent*) const;

    int getNumSelected() const      { return selectedNotes.size() + selectedSysexes.size() + selectedControllers.size(); }
    bool anythingSelected() const   { return getNumSelected() != 0; }

    const yup::Array<MidiNote*>& getSelectedNotes() const noexcept                     { return selectedNotes; }
    const yup::Array<MidiSysexEvent*>& getSelectedSysexes() const noexcept             { return selectedSysexes; }
    const yup::Array<MidiControllerEvent*>& getSelectedControllers() const noexcept    { return selectedControllers; }

    //==============================================================================
    void moveEvents (TimeDuration deltaStart, TimeDuration deltaLength, int deltaNote);
    void setNoteLengths (BeatDuration newLength);
    void setVelocities (int newVelocity);
    void changeColor (uint8_t newColor);

    void nudge (TimecodeSnapType, int leftRight, int upDown);

    TimeRange getSelectedRange() const;

    //==============================================================================
    yup::String getSelectableDescription() override;
    void selectionStatusChanged (bool isNowSelected) override;

    const yup::Array<MidiClip*>& getClips()                                            { return clips; }

    void setClips (yup::Array<MidiClip*> clips);

    /** Moves all controller data in 'clips' between edit times. Optionally moves the data making
        a copy at the original location
     */
    static void moveControllerData (const yup::Array<MidiClip*>& clips, const yup::Array<MidiControllerEvent*>* onlyTheseEvents,
                                    BeatDuration deltaBeats, TimePosition startTime, TimePosition endTime, bool makeCopy);

    /** Host should set this callback to specify if it wants MIDI CC locked to MIDI notes when
        nudging
     */
    std::function<bool()> shouldLockControllerToNotes;

private:
    yup::Array<MidiClip*> clips;

    yup::Array<MidiNote*> selectedNotes;
    yup::Array<MidiSysexEvent*> selectedSysexes;
    yup::Array<MidiControllerEvent*> selectedControllers;

    void sendSelectionChangedMessage (SelectionManager*);

    bool contains (const MidiNote&) const noexcept;
    bool contains (const MidiSysexEvent&) const noexcept;
    bool contains (const MidiControllerEvent&) const noexcept;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelectedMidiEvents)
};

} // namespace tracktion::inline engine
