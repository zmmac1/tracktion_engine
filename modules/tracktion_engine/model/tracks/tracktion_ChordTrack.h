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
class ChordTrack  : public ClipTrack
{
public:
    ChordTrack (Edit&, const yup::ValueTree&);
    ~ChordTrack() override;

    bool isChordTrack() const override                  { return true; }
    yup::String getSelectableDescription() override    { return TRANS("Chord Track"); }
    bool canContainPlugin (Plugin*) const override      { return false; }

    yup::String getTrackWarning() const;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordTrack)
};

} // namespace tracktion::inline engine
