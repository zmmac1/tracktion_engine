/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

MarkerClip::MarkerClip (const yup::ValueTree& v, EditItemID id, ClipOwner& targetParent)
   : Clip (v, targetParent, id, Type::marker)
{
}

MarkerClip::~MarkerClip()
{
    notifyListenersOfDeletion();
}

void MarkerClip::initialise()
{
    Clip::initialise();

    markerID.referTo (state, IDs::markerID, &edit.getUndoManager());

    if (! state.hasProperty (IDs::markerID))
        markerID = edit.getMarkerManager().getNextUniqueID();

    if (clipName == TRANS("New Marker"))
        clipName = TRANS("Marker") + " " + yup::String (markerID);

    speedRatio = 1.0; // not used
}

yup::String MarkerClip::getSelectableDescription()
{
    return TRANS("Marker Clip") + " - \"" + getName() + "\"";
}

yup::Color MarkerClip::getDefaultColor() const
{
    if (isSyncAbsolute())   return yup::Colors::red.withHue (0.0f);
    if (isSyncBarsBeats())  return yup::Colors::red.withHue (1.0f / 9.0f);

    return MarkerClip::getDefaultColor();
}

void MarkerClip::setMarkerID (int newID)
{
    if (getName() == (TRANS("Marker") + " " + yup::String (markerID)))
        setName (TRANS("Marker") + " " + yup::String (newID));

    markerID = newID;
}

//==============================================================================
bool MarkerClip::canBeAddedTo (ClipOwner& co)
{
    return isMarkerTrack (co);
}

void MarkerClip::valueTreePropertyChanged (yup::ValueTree& v, const yup::Identifier& i)
{
    if (v == state)
    {
        if (i == IDs::markerID)
        {
            markerID.forceUpdateOfCachedValue();
            changed();
        }
        else if (i == IDs::sync)
        {
            SelectionManager::refreshAllPropertyPanels();
            changed();
        }
    }

    Clip::valueTreePropertyChanged (v, i);
}

} // namespace tracktion::inline engine
