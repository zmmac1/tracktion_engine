/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

ExternalController::ExternalController (Engine& e, ControlSurface* c)  : engine (e), controlSurface (c)
{
    CRASH_TRACER
    controlSurface->owner = this;

    auto& cs = getControlSurface();
    auto& storage = engine.getPropertyStorage();

    maxTrackNameChars = cs.numCharactersForTrackNames;
    needsBackChannel = cs.needsMidiBackChannel;
    needsChannel = cs.needsMidiChannel;
    needsOSC = cs.needsOSCSocket;
    wantsClock = cs.wantsClock;
    followsTrackSelection = cs.followsTrackSelection;
    deletable = cs.deletable;
    auxBank = cs.wantsAuxBanks || cs.auxMode == AuxPosition::byPosition ? 0 : -1;
    allowBankingOffEnd = cs.allowBankingOffEnd;

    numDevices = engine.getPropertyStorage().getPropertyItem (SettingID::externControlNum, getName(), 1);
    mainDevice = engine.getPropertyStorage().getPropertyItem (SettingID::externControlMain, getName(), 0);

    inputDeviceNames.add (storage.getPropertyItem (SettingID::externControlIn, getName()));
    outputDeviceNames.add (storage.getPropertyItem (SettingID::externControlOut, getName()));

    for (int i = 1; i < maxDevices; i++)
    {
        inputDeviceNames.add (storage.getPropertyItem (SettingID::externControlIn, getName() + yup::String (i)));
        outputDeviceNames.add (storage.getPropertyItem (SettingID::externControlOut, getName() + yup::String (i)));
    }

    inputDevices.resize ((size_t) maxDevices);
    outputDevices.resize ((size_t) maxDevices);

    oscInputPort            = storage.getPropertyItem (SettingID::externOscInputPort, getName());
    oscOutputPort           = storage.getPropertyItem (SettingID::externOscOutputPort, getName());
    oscOutputAddr           = storage.getPropertyItem (SettingID::externOscOutputAddr, getName());

    showTrackSelection      = storage.getPropertyItem (SettingID::externControlShowSelection, getName());
    showClipSlotSelection   = storage.getPropertyItem (SettingID::externControlShowClipSlotSelection, getName());
    selectionColor         = yup::Color::fromString (storage.getPropertyItem (SettingID::externControlSelectionColor, getName(),
                                                                          yup::Colors::red.withHue (0.0f).withSaturation (0.7f).toString()).toString());
    enabled                 = storage.getPropertyItem (SettingID::externControlEnable, getName());

    midiInOutDevicesChanged();
    oscSettingsChanged();

    cs.initialiseDevice (isEnabled());
    if (numDevices != 1)
        cs.numExtendersChanged (numDevices - 1, mainDevice);

    updateDeviceState();
    changeParamBank (0);

    auto& dm = engine.getDeviceManager();
    dm.addChangeListener (this);
}

ExternalController::~ExternalController()
{
    CRASH_TRACER

    if (auto af = getCurrentPlugin())
        for (auto p : af->getAutomatableParameters())
            p->removeListener (this);

    auto& dm = engine.getDeviceManager();
    dm.removeChangeListener (this);

    for (auto& d : dm.getMidiInDevices())
        if (auto min = dynamic_cast<PhysicalMidiInputDevice*> (d.get()))
            if (min->isEnabled())
                min->removeExternalController (this);

    getControlSurface().shutDownDevice();
    controlSurface = nullptr;

    lastRegisteredListener.reset();
    lastRegisteredSelectable = nullptr;
}

void ExternalController::changeListenerCallback (yup::ChangeBroadcaster*)
{
    hasMidiInput = {};
}

yup::String ExternalController::getName() const
{
    if (auto cs = controlSurface.get())
        return cs->deviceDescription;

    return {};
}

bool ExternalController::wantsDevice (const MidiID& m)
{
    if (auto cs = controlSurface.get())
        return cs->wantsDevice (m);

    return false;
}

yup::String ExternalController::getDesiredMidiChannel() const
{
    if (auto cs = controlSurface.get())
        return cs->midiChannelName;

    return {};
}

yup::String ExternalController::getDesiredMidiBackChannel() const
{
    if (auto cs = controlSurface.get())
        return cs->midiBackChannelName;

    return {};
}

Plugin* ExternalController::getCurrentPlugin() const
{
    return dynamic_cast<Plugin*> (currentParamSource.get());
}

void ExternalController::currentEditChanged (Edit* edit)
{
    if (controlSurface != nullptr)
    {
        CRASH_TRACER
        getControlSurface().currentEditChanged (edit);
    }
}

void ExternalController::currentSelectionManagerChanged (SelectionManager* sm)
{
    if (controlSurface != nullptr)
    {
        CRASH_TRACER
        getControlSurface().currentSelectionManagerChanged (sm);
    }
}

bool ExternalController::isEnabled() const
{
    if (needsChannel)
    {
        if (hasMidiInput.has_value())
            return *hasMidiInput;

        hasMidiInput = getMidiInputDevice (0).isNotEmpty();

        return *hasMidiInput;
    }

    return enabled;
}

void ExternalController::setEnabled (bool e)
{
    if (controlSurface != nullptr && ! needsChannel)
    {
        CRASH_TRACER
        enabled = e;

        engine.getPropertyStorage().setPropertyItem (SettingID::externControlEnable, getName(), e);

        getControlSurface().initialiseDevice (isEnabled());
        updateDeviceState();
        changeParamBank (0);
    }
}

int ExternalController::getNumDevices() const
{
    return numDevices;
}

void ExternalController::setNumDevices (int num)
{
    numDevices = yup::jlimit (1, maxDevices, num);
    mainDevice = yup::jlimit (0, numDevices - 1, num);

    controlSurface->numExtendersChanged (num - 1, mainDevice);

    engine.getPropertyStorage().setPropertyItem (SettingID::externControlNum, getName(), num);
}

int ExternalController::getMainDevice() const
{
    return mainDevice;
}

void ExternalController::setMainDevice (int num)
{
    mainDevice = yup::jlimit (0, numDevices - 1, num);

    controlSurface->numExtendersChanged (num - 1, mainDevice);

    engine.getPropertyStorage().setPropertyItem (SettingID::externControlMain, getName(), mainDevice);
}

yup::String ExternalController::getMidiInputDevice (int idx) const
{
    auto name = inputDeviceNames[idx];

    if (name.isNotEmpty() && getMidiInputPorts().contains (name))
        return name;

    return {};
}

void ExternalController::setMidiInputDevice (int idx, const yup::String& nameOfMidiInput)
{
    CRASH_TRACER

    if (nameOfMidiInput.isNotEmpty())
        for (auto c : getExternalControllerManager().getControllers())
            for (int i = 0; i < maxDevices; i++)
                if (c != this && c->getMidiInputDevice (idx) == nameOfMidiInput)
                    c->setMidiInputDevice (idx, {});

    hasMidiInput = {};
    inputDeviceNames.set (idx, nameOfMidiInput);
    engine.getPropertyStorage().setPropertyItem (SettingID::externControlIn,
                                                 getName() + (idx > 0 ? yup::String (idx) : yup::String()),
                                                 nameOfMidiInput);

    midiInOutDevicesChanged();
}

yup::String ExternalController::getBackChannelDevice (int idx) const
{
    auto name = outputDeviceNames[idx];

    if (name.isNotEmpty() && getMidiOutputPorts().contains (name))
        return name;

    return {};
}

void ExternalController::deleteController()
{
    if (controlSurface != nullptr)
        getControlSurface().deleteController();
}

yup::Range<int> ExternalController::getActiveChannels() const noexcept
{
    return { channelStart, channelStart + getNumFaderChannels() };
}

yup::Range<int> ExternalController::getActiveParams() const noexcept
{
    return { startParamNumber, startParamNumber + getNumParameterControls() };
}

int ExternalController::getFaderIndexInActiveRegion (int i) const noexcept
{
    i -= channelStart;
    return yup::isPositiveAndBelow (i, getNumFaderChannels()) ? i : -1;
}

int ExternalController::getNumFaderChannels() const noexcept
{
    if (auto cs = controlSurface.get())
        return cs->numberOfFaderChannels;

    return 0;
}

int ExternalController::getNumParameterControls() const noexcept
{
    if (auto cs = controlSurface.get())
        return cs->numParameterControls;

    return 0;
}

void ExternalController::midiInOutDevicesChanged()
{
    if (! needsMidiChannel())
        return;

    auto& dm = engine.getDeviceManager();

    auto oldInputDevices = inputDevices;
    auto oldOutputDevices = outputDevices;

    for (auto& i : inputDevices)
        i = nullptr;

    for (auto& d : dm.getMidiInDevices())
    {
        CRASH_TRACER

        if (auto min = dynamic_cast<PhysicalMidiInputDevice*> (d.get()))
        {
            if (min->isEnabled())
            {
                bool used = false;

                for (int j = 0; j < numDevices; j++)
                {
                    if (min->getName().equalsIgnoreCase (inputDeviceNames[j]))
                    {
                        inputDevices[(size_t) j] = min;
                        used = true;
                    }
                }

                if (used)
                    min->setExternalController (this);
                else
                    min->removeExternalController (this);
            }
        }
    }

    for (auto& o : outputDevices)
        o = nullptr;

    for (int i = dm.getNumMidiOutDevices(); --i >= 0;)
    {
        CRASH_TRACER
        auto mo = dm.getMidiOutDevice (i);

        bool used = false;
        for (int j = 0; j < numDevices; j++)
        {
            if (mo != nullptr && mo->isEnabled() && mo->getName().equalsIgnoreCase (outputDeviceNames[j]))
            {
                outputDevices[(size_t) j] = mo;
                mo->setSendControllerMidiClock (wantsClock);
                used = true;
            }
        }

        if (used)
            mo->setExternalController (this);
        else
            mo->removeExternalController (this);
    }

    if (oldInputDevices != inputDevices || oldOutputDevices != outputDevices)
        startTimer (100);
}

void ExternalController::timerCallback()
{
    stopTimer();

    CRASH_TRACER
    if (controlSurface != nullptr)
        getControlSurface().initialiseDevice (isEnabled());

    updateDeviceState();
    changeParamBank (0);
}

void ExternalController::oscSettingsChanged()
{
    if (! needsOSCSocket())
        return;

    CRASH_TRACER
    if (controlSurface != nullptr)
        getControlSurface().initialiseDevice (isEnabled());

    getControlSurface().updateOSCSettings (oscInputPort, oscOutputPort, oscOutputAddr);

    updateDeviceState();
    changeParamBank (0);
}

void ExternalController::setBackChannelDevice (int idx, const yup::String& nameOfMidiOutput)
{
    CRASH_TRACER

    if (nameOfMidiOutput.isNotEmpty())
    {
        for (auto c : getExternalControllerManager().getControllers())
            for (int i = 0; i < maxDevices; i++)
                if (c != this && c->getBackChannelDevice (i) == nameOfMidiOutput)
                    c->setBackChannelDevice (i, {});
    }

    outputDeviceNames.set (idx, nameOfMidiOutput);
    engine.getPropertyStorage().setPropertyItem (SettingID::externControlOut,
                                                 getName() + (idx > 0 ? yup::String (idx) : yup::String()),
                                                 nameOfMidiOutput);

    midiInOutDevicesChanged();
}

bool ExternalController::isUsingMidiOutputDevice (const MidiOutputDevice* d) const noexcept
{
    for (auto od : outputDevices)
        if (od == d)
            return true;

    return false;
}

void ExternalController::setOSCInputPort (int port)
{
    oscInputPort = port;

    engine.getPropertyStorage().setPropertyItem (SettingID::externOscInputPort, getName(), oscInputPort);
    oscSettingsChanged();
}

void ExternalController::setOSCOutputPort (int port)
{
    oscOutputPort = port;

    engine.getPropertyStorage().setPropertyItem (SettingID::externOscOutputPort, getName(), oscOutputPort);
    oscSettingsChanged();
}

void ExternalController::setOSCOutputAddress (const yup::String addr)
{
    oscOutputAddr = addr;

    engine.getPropertyStorage().setPropertyItem (SettingID::externOscOutputAddr, getName(), oscOutputAddr);
    oscSettingsChanged();
}

void ExternalController::setSelectionColor (yup::Color c)
{
    if (selectionColor != c)
    {
        selectionColor = c;
        engine.getPropertyStorage().setPropertyItem (SettingID::externControlSelectionColor, getName(), c.toString());
        getControlSurface().changed();
    }
}

void ExternalController::setShowTrackSelectionColor (bool b)
{
    if (showTrackSelection != b)
    {
        showTrackSelection = b;
        engine.getPropertyStorage().setPropertyItem (SettingID::externControlShowSelection, getName(), b);
        getControlSurface().changed();
    }
}

void ExternalController::setShowClipSlotSelectionColor (bool b)
{
    if (showClipSlotSelection != b)
    {
        showClipSlotSelection = b;
        engine.getPropertyStorage().setPropertyItem (SettingID::externControlShowClipSlotSelection, getName(), b);
        getControlSurface().changed();
    }
}

void ExternalController::moveFader (int channelNum, float newSliderPos)
{
    int i = getFaderIndexInActiveRegion (channelNum);

    if (i >= 0)
        getControlSurface().moveFader (i, newSliderPos);
}

void ExternalController::moveMasterFader (float newPos)
{
    CRASH_TRACER
    if (controlSurface != nullptr)
        getControlSurface().moveMasterLevelFader (newPos);
}

void ExternalController::movePanPot (int channelNum, float newPan)
{
    int i = getFaderIndexInActiveRegion (channelNum);

    if (i >= 0)
        getControlSurface().movePanPot (i, newPan);
}

void ExternalController::moveMasterPanPot (float newPan)
{
    getControlSurface().moveMasterPanPot (newPan);
}

void ExternalController::updateSoloAndMute (int channelNum, Track::MuteAndSoloLightState state, bool isBright)
{
    int i = getFaderIndexInActiveRegion (channelNum);

    if (i >= 0)
        getControlSurface().updateSoloAndMute (i, state, isBright);
}

void ExternalController::soloCountChanged (bool anySoloTracks)
{
    if (controlSurface != nullptr)
        getControlSurface().soloCountChanged (anySoloTracks);
}

void ExternalController::playStateChanged (bool isPlaying)
{
    if (controlSurface != nullptr)
        getControlSurface().playStateChanged (isPlaying);
}

void ExternalController::recordStateChanged (bool isRecording)
{
    if (controlSurface != nullptr)
        getControlSurface().recordStateChanged (isRecording);
}

void ExternalController::automationModeChanged (bool isReading, bool isWriting)
{
    if (controlSurface != nullptr)
    {
        getControlSurface().automationReadModeChanged (isReading);
        getControlSurface().automationWriteModeChanged (isWriting);
    }
}

void ExternalController::snapChanged (bool isOn)
{
    if (controlSurface != nullptr)
        getControlSurface().snapOnOffChanged (isOn);
}

void ExternalController::loopChanged (bool isOn)
{
    if (controlSurface != nullptr)
        getControlSurface().loopOnOffChanged (isOn);
}

void ExternalController::clickChanged (bool isOn)
{
    if (controlSurface != nullptr)
        getControlSurface().clickOnOffChanged (isOn);
}

void ExternalController::channelLevelChanged (int channelNum, std::span<const float> levels)
{
    int i = getFaderIndexInActiveRegion (channelNum);

    if (i >= 0)
        getControlSurface().channelLevelChanged (i, levels);
}

void ExternalController::masterLevelsChanged (std::span<const float> levels)
{
    if (controlSurface != nullptr)
        getControlSurface().masterLevelsChanged (levels);
}

void ExternalController::timecodeChanged (int barsOrHours,
                                          int beatsOrMinutes,
                                          int ticksOrSeconds,
                                          int millisecs,
                                          bool isBarsBeats,
                                          bool isFrames)
{
    if (controlSurface != nullptr)
        getControlSurface().timecodeChanged (barsOrHours, beatsOrMinutes, ticksOrSeconds,
                                             millisecs, isBarsBeats, isFrames);
}

void ExternalController::trackSelected (int channelNum, bool isSelected)
{
    int i = getFaderIndexInActiveRegion (channelNum);

    if (i >= 0)
        getControlSurface().trackSelectionChanged (i, isSelected);
}

void ExternalController::selectOtherObject (SelectableClass::Relationship relationship, bool moveFromCurrentPlugin)
{
    if (auto sm = getExternalControllerManager().getSelectionManager())
    {
        if (moveFromCurrentPlugin
             && currentParamSource != nullptr
             && ! sm->isSelected (currentParamSource))
        {
            sm->selectOnly (currentParamSource);
        }

        sm->selectOtherObjects (relationship, false);
    }
}

void ExternalController::muteOrUnmutePlugin()
{
    if (auto p = getCurrentPlugin())
        p->setEnabled (! p->isEnabled());
}

void ExternalController::changePluginPreset (int delta)
{
    if (auto ep = dynamic_cast<ExternalPlugin*> (getCurrentPlugin()))
        if (ep->getNumPrograms() > 1)
            ep->setCurrentProgram (yup::jlimit (0,
                                                 ep->getNumPrograms() - 1,
                                                 ep->getCurrentProgram() + delta),
                                   true);
}

void ExternalController::soloPluginTrack()
{
    if (auto p = getCurrentPlugin())
        if (auto t = p->getOwnerTrack())
            t->setSolo (! t->isSolo (false));
}

void ExternalController::muteOrUnmutePluginsInTrack()
{
        if (auto p = getCurrentPlugin())
            if (auto t = p->getOwnerTrack())
                t->flipAllPluginsEnablement();
}

void ExternalController::changeFaderBank (int delta, bool moveSelection)
{
    if (controlSurface != nullptr)
    {
        if (getEdit() != nullptr)
        {
            CRASH_TRACER
            yup::SortedSet<int> selectedChannels;

            auto& ecm = getExternalControllerManager();

            for (int i = channelStart; i < (channelStart + getNumFaderChannels()); ++i)
                selectedChannels.add(i);

            channelStart = std::min (yup::jlimit (0, 127, channelStart + delta),
                                     std::max (0, ecm.getNumChannelTracks()
                                                    - (allowBankingOffEnd ? 1 : getNumFaderChannels())));

            for (int i = channelStart; i < (channelStart + getNumFaderChannels()); ++i)
            {
                if (selectedChannels.contains(i))
                    selectedChannels.removeValue(i);
                else
                    selectedChannels.add(i);
            }

            updateDeviceState();

            if (selectedChannels.size() > 0 && getShowTrackSelectionColor() && isEnabled())
                for (int i = 0; i < selectedChannels.size(); ++i)
                    ecm.repaintTrack (selectedChannels[i]);

            if (moveSelection)
            {
                if (auto sm = ecm.getSelectionManager())
                    if (auto t = ecm.getChannelTrack (channelStart))
                        if (! sm->isSelected (t) || sm->getNumObjectsSelected() != 1)
                            sm->selectOnly (t);
            }
        }
    }
}

void ExternalController::changePadBank (int delta)
{
    if (controlSurface != nullptr)
    {
        padStart = std::max (0, padStart + delta);
        updatePadColors();

        auto& ecm = getExternalControllerManager();

        if (getShowClipSlotSelectionColor() && isEnabled())
        {
            for (int i = channelStart; i < (channelStart + getNumFaderChannels()); ++i)
                ecm.repaintSlots (i);
        }
    }
}

void ExternalController::changeParamBank (int delta)
{
    if (controlSurface != nullptr)
    {
        CRASH_TRACER
        startParamNumber += delta;
        updateParamList();
        updateParameters();
    }
}

void ExternalController::updateParamList()
{
    CRASH_TRACER

    if (controlSurface != nullptr)
    {
        currentParams.clear();

        if (auto plugin = getCurrentPlugin())
        {
            auto params (plugin->getFlattenedParameterTree());
            AutomatableParameter::Array possibleParams;

           #if TRACKTION_ENABLE_CONTROL_SURFACES
            if ((getControlSurfaceIfType<NovationRemoteSl>() != nullptr
                   || getControlSurfaceIfType<RemoteSLCompact>() != nullptr)
                  && dynamic_cast<ExternalPlugin*> (plugin) != nullptr)
            {
                for (int i = 0; i < 6; ++i)
                    possibleParams.add (nullptr);
            }
            else
           #endif
            {
                if (getControlSurface().wantsDummyParams)
                    for (int i = 0; i < 2; ++i)
                        possibleParams.add (nullptr);
            }

            for (auto p : params)
                possibleParams.add (p);

            if (controlSurface != nullptr)
            {
                startParamNumber = yup::jlimit (0,
                                                 std::max (0, possibleParams.size() - getControlSurface().numParameterControls),
                                                 startParamNumber);

                for (int i = 0; i < getControlSurface().numParameterControls && i + startParamNumber < possibleParams.size(); ++i)
                    currentParams.add (possibleParams[startParamNumber + i]);
            }
        }
    }
}

void ExternalController::userMovedParameterControl (int paramNumber, float newValue, bool delta)
{
    if (auto p = currentParams[paramNumber])
    {
        if (delta)
            p->midiControllerMoved (std::clamp (p->getCurrentNormalisedValue() + newValue, 0.0f, 1.0f));
        else
            p->midiControllerMoved (newValue);
    }
}

void ExternalController::userPressedParameterControl (int paramNumber)
{
    if (auto p = currentParams[paramNumber])
        p->midiControllerPressed();
}

void ExternalController::userMovedParameterControl (int paramNumber, bool touch)
{
    if (auto p = currentParams[paramNumber])
    {
        if (touch)
            p->parameterChangeGestureBegin();
        else
            p->parameterChangeGestureEnd();
    }
}

void ExternalController::userPressedGoToMarker (int marker)
{
    if (auto tc = getTransport())
        if (auto ed = getEdit())
            if (auto mc = ed->getMarkerManager().getMarkers().getObjectPointer (marker + startMarkerNumber))
                tc->setPosition (mc->getPosition().getStart());
}

void ExternalController::updateParameters()
{
    CRASH_TRACER

    if (controlSurface == nullptr || ! isEnabled())
        return;

    auto& cs = getControlSurface();

    if (lastRegisteredSelectable != currentParamSource)
    {
        if (auto* p = getCurrentPlugin())
            for (auto* param : p->getAutomatableParameters())
                param->removeListener (this);

        if (cs.showingPluginParams())
            repaintParamSource();

        currentParamSource = lastRegisteredSelectable;
        updateParamList();

        if (cs.showingPluginParams())
            repaintParamSource();

        if (auto* p = getCurrentPlugin())
            for (auto* param : p->getAutomatableParameters())
                param->addListener (this);
    }

    auto numAvailableParams = currentParams.size();

    auto paramSourcePlugin = getCurrentPlugin();
    cs.pluginBypass (paramSourcePlugin != nullptr && ! paramSourcePlugin->isEnabled());

    if (numAvailableParams > 0)
    {
        for (int i = 0; i < numAvailableParams; ++i)
        {
            ParameterSetting param;

            if (auto p = currentParams[i])
            {
                auto pn = p->getParameterShortName (cs.numCharactersForParameterLabels);

                if (pn.length() > cs.numCharactersForParameterLabels)
                    pn = shortenName (pn, 7);

                pn.copyToUTF8 (param.label, (size_t) std::min (cs.numCharactersForParameterLabels,
                                                               (int) sizeof (param.label) - 1));

                param.value = yup::jlimit (0.0f, 1.0f, p->valueRange.convertTo0to1 (p->getCurrentBaseValue()));

                auto s = p->getLabelForValue (p->getCurrentBaseValue());

                if (s.isEmpty())
                    s = p->getCurrentValueAsString();

                if (s.length() > 6)
                {
                    if (s.toLowerCase().endsWith(" dB"))
                        s = s.dropLastCharacters(3);
                    else if (s.toLowerCase().endsWith("dB"))
                        s = s.dropLastCharacters(2);
                }

                if (s.length() > cs.numCharactersForParameterLabels)
                    s = shortenName (s, 7);

                if (s.length() < 6)
                    s = yup::String ("       ").substring (0, (7 - s.length()) / 2) + s;

                s.copyToUTF8 (param.valueDescription, 6);

                cs.parameterChanged (i, param);
            }
            else
            {
                param.label[0] = 0;
                param.valueDescription[0] = 0;
                param.value = 0.0f;

                if (auto plugin = getCurrentPlugin())
                {
                    auto t = plugin->getOwnerTrack();

                    if (startParamNumber + i == 0)
                    {
                        if (t != nullptr)
                            shortenName (t->getName(), 7)
                                .copyToUTF8 (param.label, (size_t) std::min (cs.numCharactersForParameterLabels,
                                                                             (int) sizeof (param.label) - 1));

                        cs.parameterChanged (i, param);
                    }
                    else if (startParamNumber + i == 1)
                    {
                        shortenName (plugin->getName(), 7)
                            .copyToUTF8 (param.label, (size_t) std::min (cs.numCharactersForParameterLabels,
                                                                         (int) sizeof (param.label) - 1));

                        cs.parameterChanged (i, param);
                    }
                    else
                    {
                        cs.clearParameter(i);
                    }
                }
            }
        }
    }
    else
    {
        startParamNumber = 0;
    }

    for (int i = numAvailableParams; i < cs.numParameterControls; ++i)
        cs.clearParameter (i);
}

void ExternalController::selectedPluginChanged()
{
    if (controlSurface != nullptr)
    {
        if (getControlSurface().canChangeSelectedPlugin() || lastRegisteredSelectable == nullptr)
        {
            lastRegisteredSelectable = nullptr;

            if (auto sm = getExternalControllerManager().getSelectionManager())
                lastRegisteredSelectable = sm->getSelectedObject (0);

            resetSelectableListener (lastRegisteredListener, lastRegisteredSelectable.get(), *this);

            yup::String pluginName;
            if (auto plugin = dynamic_cast<Plugin*> (lastRegisteredSelectable.get()))
                pluginName = plugin->getName();

            getControlSurface().currentSelectionChanged (pluginName);
            updateParameters();
            updateTrackSelectLights();
        }
    }
}

void ExternalController::selectableObjectChanged (Selectable*)
{
    updateParameters();
    updateMarkers();
}

void ExternalController::selectableObjectAboutToBeDeleted (Selectable* s)
{
    if (currentParamSource == s)
    {
        if (auto* p = getCurrentPlugin())
            for (auto* param : p->getAutomatableParameters())
                param->removeListener (this);

        currentParamSource = nullptr;
        updateParamList();
    }

    if (lastRegisteredSelectable == s)
    {
        lastRegisteredListener.reset();
        lastRegisteredSelectable = nullptr;

        updateParameters();
    }
}

void ExternalController::curveHasChanged (AutomatableParameter&)
{
    updateParams = true;
    triggerAsyncUpdate();
}

void ExternalController::currentValueChanged (AutomatableParameter&)
{
    updateParams = true;
    triggerAsyncUpdate();
}

void ExternalController::updateTrackSelectLights()
{
    for (int chan = channelStart; chan < (channelStart + getNumFaderChannels()); ++chan)
    {
        bool selected = false;

        if (getEdit() != nullptr)
            if (auto sm = getExternalControllerManager().getSelectionManager())
                if (auto t = getExternalControllerManager().getChannelTrack (chan))
                    selected = sm->isSelected (t);

        trackSelected (chan, selected);
    }
}

void ExternalController::updateTrackRecordLights()
{
    if (auto ed = getEdit())
    {
        auto& ecm = getExternalControllerManager();

        for (int chan = channelStart; chan < (channelStart + getNumFaderChannels()); ++chan)
        {
            if (auto t = ecm.getChannelTrack (chan))
            {
                bool isRecording = false;

                for (auto in : ed->getAllInputDevices())
                {
                    if (auto at = dynamic_cast<AudioTrack*> (t))
                    {
                        if (in->isRecordingActive (at->itemID) && in->getTargets().contains (at->itemID))
                        {
                            isRecording = true;
                            break;
                        }
                    }
                }

                getControlSurface().trackRecordEnabled (chan - channelStart, isRecording);
            }
            else
            {
                getControlSurface().trackRecordEnabled (chan - channelStart, false);
            }
        }
    }
    else
    {
        for (int chan = channelStart; chan < (channelStart + getNumFaderChannels()); ++chan)
            getControlSurface().trackRecordEnabled (chan - channelStart, false);
    }
}

void ExternalController::updatePunchLights()
{
    if (auto ed = getEdit())
        if (auto cs = controlSurface.get())
            cs->punchOnOffChanged (ed->recordingPunchInOut);
}

void ExternalController::updateScrollLights()
{
    if (auto cs = controlSurface.get())
        cs->scrollOnOffChanged (AppFunctions::isScrolling());
}

void ExternalController::updateUndoLights()
{
    if (auto ed = getEdit())
        if (auto cs = controlSurface.get())
            cs->undoStatusChanged (ed->getUndoManager().canUndo(),
                                   ed->getUndoManager().canRedo());
}

void ExternalController::clearPadColors()
{
    auto& cs = getControlSurface();

    if (cs.numberOfTrackPads > 0)
    {
        for (auto track = 0; track < cs.numberOfFaderChannels; track++)
        {
            cs.clipsPlayingStateChanged (track, false);
            for (auto scene = 0; scene < cs.numberOfTrackPads; scene++)
                cs.padStateChanged (track, scene, 0, 0);
        }
    }
}

void ExternalController::updatePadColors()
{
    auto& ecm = getExternalControllerManager();
    auto& cs = getControlSurface();

    if (cs.numberOfTrackPads > 0)
    {
        for (auto track = 0; track < cs.numberOfFaderChannels; track++)
        {
            {
                bool isPlaying = false;

                if (auto at = dynamic_cast<AudioTrack*> (ecm.getChannelTrack (track + channelStart)))
                {
                    for (auto slot : at->getClipSlotList().getClipSlots())
                    {
                        if (auto dest = slot->getInputDestination(); dest && dest->input.isRecording (dest->targetID))
                            isPlaying = true;

                        if (auto c = slot->getClip())
                            if (auto lh = c->getLaunchHandle())
                                if (lh->getPlayingStatus() == LaunchHandle::PlayState::playing || lh->getQueuedStatus() == LaunchHandle::QueueState::playQueued)
                                    isPlaying = true;

                        if (isPlaying)
                            break;
                    }
                }

                cs.clipsPlayingStateChanged (track, isPlaying);
            }

            for (auto scene = 0; scene < cs.numberOfTrackPads; scene++)
            {
                auto colourIdx = 0;
                auto state = 0;

                if (auto ft = dynamic_cast<FolderTrack*> (ecm.getChannelTrack (track + channelStart)))
                {
                    for (auto at : ft->getAllAudioSubTracks (true))
                    {
                        if (auto slot = at->getClipSlotList().getClipSlots()[padStart + scene])
                        {
                            if (auto c = slot->getClip())
                            {
                                auto col = c->getColor();

                                if (! col.isTransparent())
                                {
                                    auto numColors = 19;
                                    auto newHue = col.getHue();

                                    colourIdx = cs.limitedPadColors ? 1 : yup::jlimit (0, numColors - 1, yup::roundToInt (newHue * static_cast<float> (numColors - 1) + 1.0f));
                                    break;
                                }
                            }
                        }
                    }
                }
                else if (auto at = dynamic_cast<AudioTrack*> (ecm.getChannelTrack (track + channelStart)))
                {
                    if (auto slot = at->getClipSlotList().getClipSlots()[padStart + scene])
                    {
                        if (auto dest = slot->getInputDestination(); dest && dest->input.isRecording (dest->targetID))
                        {
                            auto epc = dest->input.edit.getTransport().getCurrentPlaybackContext();
                            const auto currentTime = epc ? epc->getUnloopedPosition() : dest->input.edit.getTransport().getPosition();

                            if (currentTime < dest->input.getPunchInTime (dest->targetID))
                            {
                                colourIdx = cs.limitedPadColors ? 3 : 1;
                                state = 1;
                            }
                            else
                            {
                                colourIdx = cs.limitedPadColors ? 3 : 1;
                                state = 2;
                            }
                        }
                        else if (auto c = slot->getClip())
                        {
                            auto col = c->getColor();

                            if (! col.isTransparent())
                            {
                                auto numColors = 19;
                                auto newHue = col.getHue();

                                colourIdx = cs.limitedPadColors ? 1 : yup::jlimit (0, numColors - 1, yup::roundToInt (newHue * static_cast<float> (numColors - 1) + 1.0f));
                            }

                            if (auto tc = getTransport())
                            {
                                if (auto lh = c->getLaunchHandle())
                                {
                                    if (lh->getPlayingStatus() == LaunchHandle::PlayState::playing)
                                    {
                                        colourIdx = cs.limitedPadColors ? 2 : 8;
                                        state = tc->isPlaying() ? 2 : 1;
                                    }
                                    else if (lh->getQueuedStatus() == LaunchHandle::QueueState::playQueued)
                                    {
                                        colourIdx = cs.limitedPadColors ? 2 : 8;;
                                        state = 1;
                                    }
                                }
                            }
                        }
                        else if (cs.recentlyPressedPads.contains ({track + channelStart, padStart + scene}))
                        {
                            auto anyPlaying = [&]()
                            {
                                if (auto tc = getTransport())
                                {
                                    for (auto slot_ : at->getClipSlotList().getClipSlots())
                                    {
                                        if (auto c_ = slot_->getClip())
                                        {
                                            if (auto lh = c_->getLaunchHandle())
                                            {
                                                if (lh->getPlayingStatus() == LaunchHandle::PlayState::playing && tc->isPlaying())
                                                    return true;
                                                else if (lh->getQueuedStatus() == LaunchHandle::QueueState::playQueued)
                                                    return true;
                                            }
                                        }
                                    }
                                }
                                return false;
                            };

                            if (anyPlaying())
                            {
                                colourIdx = cs.limitedPadColors ? 3 : 1;
                                state = 1;
                            }
                            else
                            {
                                cs.recentlyPressedPads.erase ({track + channelStart, padStart + scene});
                            }
                        }
                    }
                }

                cs.padStateChanged (track, scene, colourIdx, state);
            }
        }
    }
}

void ExternalController::updateDeviceState()
{
    if (controlSurface != nullptr)
    {
        if (auto edit = getEdit())
        {
            auto& ecm = getExternalControllerManager();
            auto& cs = getControlSurface();

            {
                CRASH_TRACER
                bool anySoloTracks = edit->areAnyTracksSolo();

                for (int chan = channelStart; chan < (channelStart + getNumFaderChannels()); ++chan)
                {
                    if (auto t = ecm.getChannelTrack (chan))
                    {
                        auto at = dynamic_cast<AudioTrack*> (t);
                        auto ft = dynamic_cast<FolderTrack*> (t);

                        if (auto vp = at != nullptr ? at->getVolumePlugin() : nullptr)
                        {
                            moveFader (chan, vp->getSliderPos());
                            movePanPot (chan, vp->getPan());
                        }
                        else if (auto vca = ft != nullptr ? ft->getVCAPlugin() : nullptr)
                        {
                            moveFader (chan, vca->getSliderPos());
                            movePanPot (chan, 0.0f);
                        }
                        else
                        {
                            moveFader (chan, decibelsToVolumeFaderPosition (0.0f));
                            movePanPot (chan, 0.0f);
                        }

                        updateSoloAndMute (chan, t->getMuteAndSoloLightState(), true);

                        channelLevelChanged (chan, {});

                        if (auto sm = ecm.getSelectionManager())
                            trackSelected (chan, sm->isSelected (t));
                    }
                    else
                    {
                        moveFader (chan, decibelsToVolumeFaderPosition (0.0f));
                        movePanPot (chan, 0.0f);
                        updateSoloAndMute (chan, {}, false);
                        channelLevelChanged (chan, {});
                        trackSelected (chan, false);
                    }
                }

                updateTrackSelectLights();
                updateTrackRecordLights();
                soloCountChanged (anySoloTracks);
            }

            {
                CRASH_TRACER

                if (auto masterVol = edit->getMasterVolumePlugin())
                {
                    moveMasterFader (masterVol->getSliderPos());
                    moveMasterPanPot (masterVol->getPan());
                }

                yup::StringArray trackNames;

                for (int i = 0; i < getNumFaderChannels(); ++i)
                {
                    yup::String name;

                    if (auto track = ecm.getChannelTrack (i + channelStart))
                    {
                        yup::String trackName (track->getName());

                        if (trackName.startsWithIgnoreCase (TRANS("Track") + " ") && trackName.length() > maxTrackNameChars)
                            trackName = yup::String (trackName.getTrailingIntValue());
                        else if (trackName.length() > maxTrackNameChars)
                            trackName = shortenName (trackName, maxTrackNameChars);

                        name = trackName.substring (0, maxTrackNameChars);
                    }

                    trackNames.add (name);
                }

                cs.faderBankChanged (channelStart, trackNames);

                updatePadColors();

                if (cs.showingMarkers())
                    ecm.updateMarkers();
            }

            if (auto tc = getTransport())
            {
                CRASH_TRACER
                playStateChanged (tc->isPlaying());
                recordStateChanged (tc->isRecording());

                automationModeChanged (edit->getAutomationRecordManager().isReadingAutomation(),
                                       edit->getAutomationRecordManager().isWritingAutomation());

                snapChanged (tc->snapToTimecode);
                loopChanged (tc->looping);
                clickChanged (edit->clickTrackEnabled);
                cs.scrollOnOffChanged (AppFunctions::isScrolling());
                cs.punchOnOffChanged (edit->recordingPunchInOut);
                cs.slaveOnOffChanged (edit->isTimecodeSyncEnabled());

                masterLevelsChanged ({});

                updateTrackRecordLights();
                cs.auxBankChanged (auxBank);
                auxSendLevelsChanged();

                cs.updateMiscFeatures();
            }
            else
            {
                jassertfalse;
            }
        }
        else
        {
            updateTrackSelectLights();
            updateTrackRecordLights();
            clearPadColors();
        }
    }
}

void ExternalController::auxSendLevelsChanged()
{
    if (controlSurface != nullptr)
    {
        auto& ecm = getExternalControllerManager();
        auto& cs = getControlSurface();

        for (int chan = channelStart; chan < (channelStart + getNumFaderChannels()); ++chan)
        {
            if (auto t = ecm.getChannelTrack (chan))
            {
                auto at = dynamic_cast<AudioTrack*> (t);

                if (at == nullptr)
                {
                    for (auto i = 0; i < cs.numAuxes; i++)
                        cs.clearAux (chan - channelStart, i);
                }
                else if (cs.auxMode == AuxPosition::byPosition)
                {
                    for (auto i = 0; i < cs.numAuxes; i++)
                    {
                        if (auto aux = at->getAuxSendPlugin (auxBank + i, AuxPosition::byPosition))
                        {
                            auto nm = aux->getBusName();

                            if (nm.length() > cs.numCharactersForAuxLabels)
                                nm = shortenName (nm, cs.numCharactersForAuxLabels);

                            cs.moveAux (chan - channelStart, i, nm.toRawUTF8(), decibelsToVolumeFaderPosition (aux->getGainDb()));
                        }
                        else
                        {
                            cs.clearAux (chan - channelStart, i);
                        }
                    }
                }
                else
                {
                    for (auto i = 0; i < cs.numAuxes; i++)
                    {
                        if (auto aux = at->getAuxSendPlugin (auxBank + i, AuxPosition::byBus))
                        {
                            auto nm = aux->getBusName();

                            if (nm.length() > cs.numCharactersForAuxLabels)
                                nm = shortenName (nm, cs.numCharactersForAuxLabels);

                            cs.moveAux (chan - channelStart, i, nm.toRawUTF8(), decibelsToVolumeFaderPosition (aux->getGainDb()));
                        }
                        else
                        {
                            cs.clearAux (chan - channelStart, i);
                        }
                    }
                }
            }
            else
            {
                for (auto i = 0; i < cs.numAuxes; i++)
                    cs.clearAux (chan - channelStart, i);
            }
        }
    }
}

void ExternalController::acceptMidiMessage (MidiInputDevice& d, const yup::MidiMessage& m)
{
    CRASH_TRACER
    const yup::ScopedLock sl (incomingMidiLock);

    int idx = 0;

    for (size_t i = 0; i < inputDevices.size(); ++i)
        if (inputDevices[i] == &d)
            idx = (int) i;

    pendingMidiMessages.add ({ idx, m });
    processMidi = true;
    triggerAsyncUpdate();
}

bool ExternalController::wantsMessage (MidiInputDevice& d, const yup::MidiMessage& m)
{
    int idx = 0;

    for (size_t i = 0; i < inputDevices.size(); ++i)
        if (inputDevices[i] == &d)
            idx = (int) i;

    return controlSurface != nullptr && getControlSurface().wantsMessage (idx, m);
}

bool ExternalController::eatsAllMessages() const
{
    return controlSurface != nullptr && getControlSurface().eatsAllMessages();
}

void ExternalController::setEatsAllMessages (bool eatAll)
{
    if (controlSurface != nullptr)
        getControlSurface().setEatsAllMessages (eatAll);
}

void ExternalController::handleAsyncUpdate()
{
    if (processMidi)
    {
        processMidi = false;

        if (controlSurface != nullptr)
        {
            CRASH_TRACER

            yup::Array<std::pair<int, yup::MidiMessage>> messages;
            messages.ensureStorageAllocated (16);

            {
                const yup::ScopedLock sl (incomingMidiLock);
                messages.swapWith (pendingMidiMessages);
            }

            for (auto& m : messages)
                getControlSurface().acceptMidiMessage (m.first, m.second);
        }
    }

    if (updateParams)
    {
        updateParams = false;
        updateParameters();
    }
}

yup::String ExternalController::getNoDeviceSelectedMessage()
{
    return "<" + TRANS("No Device Selected") + ">";
}

yup::StringArray ExternalController::getMidiInputPorts() const
{
    CRASH_TRACER
    yup::StringArray inputNames;
    inputNames.add (getNoDeviceSelectedMessage());

    auto& dm = engine.getDeviceManager();

    auto wantsDevice = [] ([[maybe_unused]] MidiInputDevice* in)
    {
       #if ! YUP_DEBUG
        if (dynamic_cast<VirtualMidiInputDevice*> (in))
            return false;
       #endif

        return true;
    };

    for (auto& m : dm.getMidiInDevices())
        if (m->isEnabled() && wantsDevice (m.get()))
            inputNames.add (m->getName());

    return inputNames;
}

yup::StringArray ExternalController::getMidiOutputPorts() const
{
    CRASH_TRACER
    yup::StringArray outputNames;
    outputNames.add (getNoDeviceSelectedMessage());
    auto& dm = engine.getDeviceManager();

    for (int i = 0; i < dm.getNumMidiOutDevices(); ++i)
        if (auto m = dm.getMidiOutDevice (i))
            if (m->isEnabled())
                outputNames.add(m->getName());

    return outputNames;
}

bool ExternalController::shouldTrackBeColored (int channelNum)
{
    return channelNum >= channelStart
            && channelNum < (channelStart + getNumFaderChannels())
            && getControlSurface().showingTracks()
            && getShowTrackSelectionColor()
            && isEnabled();
}

void ExternalController::getTrackColor (int channelNum, yup::Color& color)
{
    if (channelNum >= channelStart
         && channelNum < channelStart + getNumFaderChannels()
         && getControlSurface().showingTracks()
         && getShowTrackSelectionColor()
         && isEnabled())
    {
        if (color.getARGB() == 0)
            color = getSelectionColor();
        else
            color = color.overlaidWith (getSelectionColor().withAlpha (0.8f));
    }
}

std::optional<ColorArea> ExternalController::getColoredArea (const Edit& e)
{
    if (&e != getEdit())
        return {};

    if (controlSurface == nullptr || controlSurface->numberOfTrackPads == 0 || ! getControlSurface().showingClipSlots() || ! getShowClipSlotSelectionColor() || ! isEnabled())
        return {};

    auto& ecm = getExternalControllerManager();

    auto t1 = ecm.getChannelTrack (channelStart);
    auto t2 = t1;

    for (auto i = 0; i < controlSurface->numberOfFaderChannels; i++)
        if (auto t = t2 = ecm.getChannelTrack (channelStart + i))
            t2 = t;

    if (t1 && t2)
    {
        ColorArea c = {getSelectionColor(), *t1, *t2, padStart, padStart + controlSurface->numberOfTrackPads - 1};
        return c;
    }

    return {};
}

bool ExternalController::shouldPluginBeColored (Plugin* p)
{
    return controlSurface != nullptr
            && (getControlSurface().isPluginSelected (p)
                 || (p == currentParamSource && getControlSurface().showingPluginParams()))
            && getShowTrackSelectionColor()
            && isEnabled();
}

void ExternalController::getPluginColor (Plugin* plugin, yup::Color& color)
{
    if (shouldPluginBeColored (plugin) && getShowTrackSelectionColor() && isEnabled())
    {
        if (color.getARGB() == 0)
            color = getSelectionColor();
        else
            color = color.overlaidWith (getSelectionColor().withAlpha (0.8f));
    }
}

void ExternalController::repaintParamSource()
{
    CRASH_TRACER

    if (auto plugin = getCurrentPlugin())
        getExternalControllerManager().repaintPlugin (*plugin);
}

void ExternalController::redrawTracks()
{
    CRASH_TRACER

    auto& ecm = getExternalControllerManager();
    auto numTracks = ecm.getNumChannelTracks();

    for (int i = 0; i < numTracks; ++i)
        ecm.repaintTrack (i);
}

void ExternalController::changeMarkerBank (int delta)
{
    if (controlSurface != nullptr)
    {
        startMarkerNumber += delta;
        updateMarkers();
    }
}

void ExternalController::updateMarkers()
{
    if (controlSurface != nullptr)
    {
        auto& cs = getControlSurface();
        int numMarkersUsed = 0;

        if (auto ed = getEdit())
        {
            auto allMarkers = ed->getMarkerManager().getMarkers();

            if (allMarkers.size() > 0)
            {
                startMarkerNumber = yup::jlimit (0,
                                                  std::max (0, allMarkers.size() - cs.numMarkers),
                                                  startMarkerNumber);

                for (int i = 0; (i < cs.numMarkers) && (i + startMarkerNumber < allMarkers.size()); ++i)
                {
                    if (auto mc = allMarkers.getObjectPointer (i + startMarkerNumber))
                    {
                        yup::String pn (mc->getName().replace ("marker", "mk", true));

                        if (pn.isEmpty())
                            pn = yup::String (mc->getMarkerID());
                        else if (pn.length() > cs.numCharactersForMarkerLabels)
                            pn = shortenName (pn, 7);

                        MarkerSetting ms;
                        pn.copyToUTF8 (ms.label, (size_t) std::min (cs.numCharactersForMarkerLabels,
                                                                    (int) sizeof (ms.label) - 1));
                        ms.number   = mc->getMarkerID();
                        ms.absolute = mc->isSyncAbsolute();

                        cs.markerChanged (i, ms);
                        numMarkersUsed = i + 1;
                    }
                }
            }
        }

        for (int i = numMarkersUsed; i < cs.numMarkers; ++i)
            cs.clearMarker(i);
    }
}

void ExternalController::changeAuxBank (int delta)
{
    if (controlSurface != nullptr)
    {
        if (getControlSurface().auxMode == AuxPosition::byPosition)
            auxBank = yup::jlimit (0, 15, auxBank + delta);
        else
            auxBank = yup::jlimit (-1, 31, auxBank + delta);

        getControlSurface().auxBankChanged (auxBank);
        auxSendLevelsChanged();
    }
}

void ExternalController::setAuxBank (int num)
{
    if (controlSurface != nullptr)
    {
        if (getControlSurface().auxMode == AuxPosition::byPosition)
            auxBank = yup::jlimit (0, 15, num);
        else
            auxBank = yup::jlimit (-1, 31, num);

        getControlSurface().auxBankChanged (auxBank);
        auxSendLevelsChanged();
    }
}

yup::String ExternalController::shortenName (yup::String s, int maxLen)
{
    if (s.length() < maxLen)
        return s;

    s = s.replace (TRANS("Track"), TRANS("Trk"));

    if (s.length() < maxLen)
        return s;

    bool hasSeenConsonant = false;
    yup::String result;

    for (int i = 0; i < s.length(); ++i)
    {
        const bool isVowel = yup::String ("aeiou").containsChar (s[i]);

        hasSeenConsonant = (hasSeenConsonant || ! isVowel)
                             && ! yup::CharacterFunctions::isWhitespace (s[i]);

        if (! (hasSeenConsonant && isVowel))
            result += s[i];
    }

    return result.substring (0, maxLen);
}

} // namespace tracktion::inline engine
