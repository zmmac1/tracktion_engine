/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

ArrangerClip::ArrangerClip (const yup::ValueTree& v, EditItemID id, ClipOwner& targetParent)
   : Clip (v, targetParent, id, Type::arranger)
{
}

ArrangerClip::~ArrangerClip()
{
    notifyListenersOfDeletion();
}

void ArrangerClip::initialise()
{
    Clip::initialise();
}

yup::String ArrangerClip::getSelectableDescription()
{
    return TRANS("Arranger Clip") + " - \"" + getName() + "\"";
}

yup::Color ArrangerClip::getDefaultColor() const
{
    return yup::Colors::red.withHue (7.0f / 9.0f);
}

//==============================================================================
bool ArrangerClip::canBeAddedTo (ClipOwner& co)
{
    return isArrangerTrack (co);
}

void ArrangerClip::valueTreePropertyChanged (yup::ValueTree& v, const yup::Identifier& i)
{
    if (v == state)
    {
        if (i == IDs::sync)
        {
            SelectionManager::refreshAllPropertyPanels();
            changed();
        }
    }

    Clip::valueTreePropertyChanged (v, i);
}

} // namespace tracktion::inline engine
