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
class MarkerTrack  : public ClipTrack
{
public:
    MarkerTrack (Edit&, const yup::ValueTree&);
    ~MarkerTrack() override;

    bool isMarkerTrack() const override;
    yup::String getSelectableDescription() override;
    yup::String getName() const override;
    bool canContainPlugin (Plugin*) const override;

private:
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MarkerTrack)
};

} // namespace tracktion::inline engine
