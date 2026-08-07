/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

AutomationTrack::AutomationTrack (Edit& e, const yup::ValueTree& v)
    : Track (e, v, false)
{
}

AutomationTrack::~AutomationTrack()
{
    notifyListenersOfDeletion();
}

//==============================================================================
yup::String AutomationTrack::getSelectableDescription()
{
    auto n = getName();
    return TRANS("Automation") + (n.isEmpty() ? yup::String() : " - \"" + getName() + "\"");
}

yup::String AutomationTrack::getName() const
{
    if (auto ap = getCurrentlyShownAutoParam())
        return ap->getFullName();

    return {};
}

} // namespace tracktion::inline engine
