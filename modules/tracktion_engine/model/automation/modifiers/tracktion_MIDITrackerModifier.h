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
class MIDITrackerModifier   : public Modifier,
                              private ValueTreeAllEventListener
{
public:
    //==============================================================================
    MIDITrackerModifier (Edit&, const yup::ValueTree&);
    ~MIDITrackerModifier() override;

    using Ptr = yup::ReferenceCountedObjectPtr<MIDITrackerModifier>;
    using Array = yup::ReferenceCountedArray<MIDITrackerModifier>;

    using Modifier::initialise;
    void initialise() override;
    yup::String getName() const override               { return TRANS("MIDI Tracker Modifier"); }

    //==============================================================================
    /** Returns the current value of the modifier. */
    float getCurrentValue() override;

    /** Returns the MIDI value, either a pitch or velocity. */
    int getCurrentMIDIValue() const noexcept;

    AutomatableParameter::ModifierAssignment* createAssignment (const yup::ValueTree&) override;

    yup::StringArray getMidiInputNames() override;
    ProcessingPosition getProcessingPosition() override { return ProcessingPosition::preFX; }

    void applyToBuffer (const PluginRenderContext&) override;

    //==============================================================================
    struct Assignment   : public AutomatableParameter::ModifierAssignment
    {
        Assignment (const yup::ValueTree&, const MIDITrackerModifier&);

        bool isForModifierSource (const ModifierSource&) const override;
        yup::ReferenceCountedObjectPtr<MIDITrackerModifier> getMIDITrackerModifier() const;

        const EditItemID midiTrackerModifierID;

        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Assignment)
    };

    //==============================================================================
    yup::String getSelectableDescription() override    { return getName(); }

    //==============================================================================
    enum Type
    {
        pitch          = 0,
        velocity       = 1
    };

    static yup::StringArray getTypeNames();

    enum Mode
    {
        absolute       = 0,
        relative       = 1
    };

    static yup::StringArray getModeNames();

    /** Returns a ValueTree representing the absolute nodes of the map.
        This will have 5 children each with a midi and value property which you can adjust.
    */
    yup::ValueTree getNodeState() const                { return nodeState; }

    juce::CachedValue<float> type, mode, relativeRoot, relativeSpread;

    AutomatableParameter::Ptr typeParam, modeParam, relativeRootParam, relativeSpreadParam;

private:
    yup::ValueTree nodeState;

    LambdaTimer changedTimer;
    std::atomic<float> currentModifierValue { 0.0f };
    std::atomic<int> currentMIDIValue { 0 };

    struct Map { std::pair<int, float> node[5]; } currentMap;
    const yup::SpinLock mapLock;

    void refreshCurrentValue();
    void updateMapFromTree();
    void updateValueFromMap (int);

    void midiEvent (const yup::MidiMessage&);

    void valueTreeChanged() override;
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
};

} // namespace tracktion::inline engine
