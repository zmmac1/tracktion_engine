/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** A track similar to the MarkerTrack but can be used to move sections of an Edit around.
*/
class ArrangerTrack  : public ClipTrack
{
public:
    ArrangerTrack (Edit&, const yup::ValueTree&);
    ~ArrangerTrack() override;

    bool isArrangerTrack() const override;
    yup::String getSelectableDescription() override;
    yup::String getName() const override;
    bool canContainPlugin (Plugin*) const override;

private:
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrangerTrack)
};

} // namespace tracktion::inline engine
