/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

ArrangerTrack::ArrangerTrack (Edit& e, const yup::ValueTree& v)
    : ClipTrack (e, v, false)
{
}

ArrangerTrack::~ArrangerTrack()
{
    notifyListenersOfDeletion();
}

bool ArrangerTrack::isArrangerTrack() const             { return true; }
yup::String ArrangerTrack::getSelectableDescription()  { return getName(); }
yup::String ArrangerTrack::getName() const             { return TRANS("Arranger"); }
bool ArrangerTrack::canContainPlugin (Plugin*) const    { return false; }

} // namespace tracktion::inline engine
