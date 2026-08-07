/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

AuxReturnPlugin::AuxReturnPlugin (PluginCreationInfo info)  : Plugin (info)
{
    busNumber.referTo (state, IDs::busNum, getUndoManager());
}

AuxReturnPlugin::~AuxReturnPlugin()
{
    notifyListenersOfDeletion();
}

const char* AuxReturnPlugin::xmlTypeName = "auxreturn";

yup::String AuxReturnPlugin::getName() const
{
    auto nm = edit.getAuxBusName (busNumber);

    if (nm.isNotEmpty())
        return "R:" + nm;

    return TRANS("Aux Return") + " #" + yup::String (busNumber + 1);
}

yup::String AuxReturnPlugin::getShortName (int)
{
    auto nm = edit.getAuxBusName (busNumber);

    if (nm.isNotEmpty())
        return "R:" + nm;

    return "Ret:" + yup::String (busNumber + 1);
}

void AuxReturnPlugin::initialise (const PluginInitialisationInfo&)
{
}

void AuxReturnPlugin::deinitialise()
{
}

void AuxReturnPlugin::applyToBuffer (const PluginRenderContext&)
{
}

void AuxReturnPlugin::restorePluginStateFromValueTree (const yup::ValueTree& v)
{
    copyPropertiesToCachedValues (v, busNumber);

    for (auto p : getAutomatableParameters())
        p->updateFromAttachedValue();
}

} // namespace tracktion::inline engine
