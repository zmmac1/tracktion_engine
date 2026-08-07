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
class ChordClip   : public Clip,
                    private yup::AsyncUpdater
{
public:
    ChordClip (const yup::ValueTree&, EditItemID, ClipOwner& targetParent);
    ~ChordClip() override;

    yup::String getSelectableDescription() override;
    bool canBeAddedTo (ClipOwner&) override;
    yup::Color getColor() const override;
    yup::Color getDefaultColor() const override;
    void initialise() override;

    bool isMidi() const override         { return false; }
    bool isMuted() const override        { return false; }

    PatternGenerator* getPatternGenerator() override;
    void pitchTempoTrackChanged() override;

protected:
    void handleAsyncUpdate() override;

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override;
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override;
    void valueTreeParentChanged (yup::ValueTree&) override;

private:

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordClip)
};

} // namespace tracktion::inline engine
