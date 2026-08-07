/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

static constexpr int maxRackAudioChans = 64;

static yup::String getDefaultInputName (int i)
{
    if (i == 0)  return TRANS("Left input");
    if (i == 1)  return TRANS("Right input");

    return TRANS("Input") + " " + yup::String (i + 1);
}

static yup::String getDefaultOutputName (int i)
{
    if (i == 0)  return TRANS("Left output");
    if (i == 1)  return TRANS("Right output");

    return TRANS("Output") + " " + yup::String (i + 1);
}

//==============================================================================
RackConnection::RackConnection (const yup::ValueTree& v, juce::UndoManager* um)
    : state (v)
{
    sourceID.referTo (state, IDs::src, um);
    destID.referTo (state, IDs::dst, um);
    sourcePin.referTo (state, IDs::srcPin, um);
    destPin.referTo (state, IDs::dstPin, um);
}

//==============================================================================
struct RackType::RackPluginList  : public ValueTreeObjectList<RackType::PluginInfo>
{
    RackPluginList (RackType& t, const yup::ValueTree& parentTree)
        : ValueTreeObjectList<PluginInfo> (parentTree), type (t)
    {
        CRASH_TRACER
        // Exception will be caught by the Edit constructor
        callBlocking ([this] { this->rebuildObjects(); });
    }

    ~RackPluginList() override
    {
        freeObjects();
    }

    bool isSuitableType (const yup::ValueTree& v) const override
    {
        return v.hasType (IDs::PLUGININSTANCE);
    }

    PluginInfo* createNewObject (const yup::ValueTree& v) override
    {
        CRASH_TRACER
        auto* i = new PluginInfo();
        i->plugin = type.edit.getPluginCache().getOrCreatePluginFor (v.getChild(0));
        i->state = v;
        return i;
    }

    void deleteObject (PluginInfo* p) override
    {
        jassert (p != nullptr);
        delete p;
    }

    void valueTreeChildAdded (yup::ValueTree& p, yup::ValueTree& tree) override
    {
        ValueTreeObjectList<RackType::PluginInfo>::valueTreeChildAdded (p, tree);

        if (tree.hasType (IDs::PLUGIN) && p.hasType (IDs::PLUGININSTANCE))
            for (auto info : objects)
                if (info->plugin == nullptr && info->state == p)
                    info->plugin = type.edit.getPluginCache().getOrCreatePluginFor (tree);
    }

    void objectRemoved (PluginInfo*) override
    {
        removeBrokenConnections (type.state, type.getUndoManager());
    }

    void newObjectAdded (PluginInfo*) override {}
    void objectOrderChanged() override {}
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override {}

    RackType& type;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RackPluginList)
};

struct RackType::ConnectionList  : public ValueTreeObjectList<RackConnection>
{
    ConnectionList (RackType& t, const yup::ValueTree& parentTree)
        : ValueTreeObjectList<RackConnection> (parentTree), type (t)
    {
        CRASH_TRACER
        rebuildObjects();
    }

    ~ConnectionList() override
    {
        freeObjects();
    }

    bool isSuitableType (const yup::ValueTree& v) const override
    {
        return v.hasType (IDs::CONNECTION);
    }

    RackConnection* createNewObject (const yup::ValueTree& v) override
    {
        if (! type.edit.isLoading())
            TRACKTION_ASSERT_MESSAGE_THREAD

        return new RackConnection (v, type.getUndoManager());
    }

    void deleteObject (RackConnection* t) override
    {
        if (! type.edit.isLoading())
            TRACKTION_ASSERT_MESSAGE_THREAD

        jassert (t != nullptr);
        delete t;
    }

    void newObjectAdded (RackConnection*) override {}
    void objectRemoved (RackConnection*) override {}
    void objectOrderChanged() override {}
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override {}

    RackType& type;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectionList)
};

//==============================================================================
struct RackType::WindowStateList  : public ValueTreeObjectList<WindowState>
{
    WindowStateList (RackType& t)
        : ValueTreeObjectList<WindowState> (t.state), type (t)
    {
        CRASH_TRACER
        // Exception will be caught by the Edit constructor
        callBlocking ([this] { this->rebuildObjects(); });
    }

    ~WindowStateList() override
    {
        freeObjects();
    }

    bool isSuitableType (const yup::ValueTree& v) const override
    {
        return v.hasType (IDs::WINDOWSTATE);
    }

    WindowState* createNewObject (const yup::ValueTree& v) override
    {
        CRASH_TRACER
        return new WindowState (type, v);
    }

    void deleteObject (WindowState* t) override
    {
        jassert (t != nullptr);
        delete t;
    }

    void newObjectAdded (WindowState*) override                                         {}
    void objectRemoved (WindowState*) override                                          {}
    void objectOrderChanged() override                                                  {}
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override  {}

    RackType& type;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowStateList)
};

//==============================================================================
RackType::RackType (Edit& ed, const yup::ValueTree& v)
    : EditItem (ed, v),
      MacroParameterElement (ed, v),
      state (v)
{
    CRASH_TRACER

    auto windowState = state.getChildWithName (IDs::WINDOWSTATE);

    if (! windowState.isValid())
    {
        auto ws = createValueTree (IDs::WINDOWSTATE,
                                   IDs::windowPos, state[IDs::windowPos]);

        state.addChild (ws, -1, nullptr);
    }

    windowStateList = std::make_unique<WindowStateList> (*this);

    rackName.referTo (state, IDs::name, getUndoManager());

    if (rackName.get().isEmpty())
        rackName = TRANS("New Rack");

    pluginList = std::make_unique<RackPluginList> (*this, state);
    connectionList = std::make_unique<ConnectionList> (*this, state);

    if (getOutputNames().isEmpty())
        addDefaultOutputs();

    if (getInputNames().isEmpty())
        addDefaultInputs();

    loadWindowPosition();
    checkConnections();

    modifierList = std::make_unique<ModifierList> (edit, state.getOrCreateChildWithName (IDs::MODIFIERS, getUndoManager()));

    state.addListener (this);
}

RackType::~RackType()
{
    CRASH_TRACER
    notifyListenersOfDeletion();
    hideWindowForShutdown();
    state.removeListener (this);
    windowStateList.reset();
}

static yup::ValueTree createInOrOut (const yup::Identifier& type, const yup::String& name)
{
    return createValueTree (type,
                            IDs::name, name);
}

void RackType::removeAllInputsAndOutputs()
{
    for (int i = state.getNumChildren(); --i >= 0;)
    {
        auto v = state.getChild (i);

        if (v.hasType (IDs::OUTPUT) || v.hasType (IDs::INPUT))
            state.removeChild (i, getUndoManager());
    }
}

void RackType::addDefaultInputs()
{
    state.addChild (createInOrOut (IDs::INPUT, "midi input"), -1, nullptr);
    state.addChild (createInOrOut (IDs::INPUT, "input 1 (left)"), -1, nullptr);
    state.addChild (createInOrOut (IDs::INPUT, "input 2 (right)"), -1, nullptr);
}

void RackType::addDefaultOutputs()
{
    state.addChild (createInOrOut (IDs::OUTPUT, "midi output"), -1, nullptr);
    state.addChild (createInOrOut (IDs::OUTPUT, "output 1 (left)"), -1, nullptr);
    state.addChild (createInOrOut (IDs::OUTPUT, "output 2 (right)"), -1, nullptr);
}

namespace
{
    bool hasAnyModifierAssignmentsRecursive (const yup::ValueTree& vt)
    {
        if (vt.hasType (IDs::MODIFIERASSIGNMENTS) && vt.getNumChildren() > 0)
            return true;

        for (auto v : vt)
            if (hasAnyModifierAssignmentsRecursive (v))
                return true;

        return false;
    }
}

yup::Result RackType::restoreStateFromValueTree (const yup::ValueTree& vt)
{
    // First check to see if the incoming state has any modifier assignments
    if (hasAnyModifierAssignmentsRecursive (vt))
        return yup::Result::fail (TRANS("Unable to apply preset due to Macro or Modifier connections, please create a new Rack from the preset"));

    auto v = vt.createCopy();

    if (v.hasType (IDs::PRESET))
        v = v.getChildWithName (IDs::RACK);

    if (! v.hasType (IDs::RACK))
        return yup::Result::fail (TRANS("Invalid or corrupted preset"));

    itemID.writeID (v, nullptr);

    {
        auto um = getUndoManager();
        state.copyPropertiesFrom (v, um);

        // Remove all children except the window state
        for (int i = state.getNumChildren(); --i >= 0;)
        {
            auto c = state.getChild (i);

            if (! (c.hasType (IDs::WINDOWSTATE) || c.hasType (IDs::MACROPARAMETERS) || c.hasType (IDs::MODIFIERS)))
                state.removeChild (i, um);
        }

        // Add all children except window state, macros and modifiers
        for (int i = v.getNumChildren(); --i >= 0;)
        {
            auto c = v.getChild (i);
            v.removeChild (i, nullptr);

            if (! (c.hasType (IDs::WINDOWSTATE) || c.hasType (IDs::MACROPARAMETERS) || c.hasType (IDs::MODIFIERS)))
                state.addChild (c, 0, um);
        }

        checkConnections();

        for (auto rf : activeRackInstances)
            rf->changed();
    }

    propertiesChanged();
    changed();

    return yup::Result::ok();
}

yup::ValueTree RackType::createStateCopy (bool includeAutomation)
{
    saveWindowPosition();

    for (auto p : getPlugins())
        p->flushPluginStateToValueTree();

    auto v = state.createCopy();

    if (! includeAutomation)
        AutomationCurve::removeAllAutomationCurvesRecursively (v);

    return v;
}

yup::Array<RackType::WindowState*> RackType::getWindowStates() const
{
    return windowStateList->objects;
}

void RackType::loadWindowPosition()
{
    for (auto* ws : getWindowStates())
    {
        if (ws->state.hasProperty (IDs::windowPos))
        {
            auto s = ws->state[IDs::windowPos].toString();

            if (s.isNotEmpty())
                ws->lastWindowBounds = yup::Rectangle<int>::fromString (s);
        }
    }
}

void RackType::saveWindowPosition()
{
    for (auto* ws : getWindowStates())
    {
        if (ws->lastWindowBounds)
        {
            auto windowState = ws->lastWindowBounds->toString();
            ws->state.setProperty (IDs::windowPos, windowState.isEmpty() ? yup::var() : yup::var (windowState), nullptr);
        }
    }
}

RackType::Ptr RackType::createTypeToWrapPlugins (const Plugin::Array& plugins, Edit& sourceEdit)
{
    auto rack = sourceEdit.getRackList().addNewRack();

    if (plugins.size() == 1)
        rack->rackName = plugins.getFirst()->getName() + " " + TRANS("Wrapper");
    else if (plugins.getFirst()->getOwnerTrack() == nullptr)
        rack->rackName = TRANS("Master Wrapper");
    else
        rack->rackName = plugins.getFirst()->getOwnerTrack()->getName() + " "  + TRANS("Wrapper");

    rack->removeInput (2);
    rack->removeInput (1);
    rack->removeOutput (2);
    rack->removeOutput (1);

    for (int i = 0; i < plugins.size(); ++i)
        if (auto f = plugins[i])
            rack->addPlugin (f, yup::Point<float> (1.0f / float (plugins.size() + 1) * float (i + 1), 0.5f), false);

    yup::StringArray ins, outs;
    plugins.getFirst()->getChannelNames (&ins, nullptr);
    plugins.getLast()->getChannelNames (nullptr, &outs);

    // connect the left side to the first plugin
    for (int i = 0; i < std::min (maxRackAudioChans, ins.size()); ++i)
    {
        auto name = ins[i];

        if (name.isEmpty() || name.equalsIgnoreCase (TRANS("Unnamed")))
            name = getDefaultInputName (i);

        rack->addInput (-1, name);
        rack->addConnection ({}, i + 1, plugins.getFirst()->itemID, i + 1);
    }

    // connect the right side to the last plugin
    for (int i = 0; i < std::min (maxRackAudioChans, outs.size()); ++i)
    {
        auto name = outs[i];

        if (name.isEmpty() || name.equalsIgnoreCase (TRANS("Unnamed")))
            name = getDefaultOutputName (i);

        rack->addOutput (-1, name);
        rack->addConnection (plugins.getLast()->itemID, i + 1, {}, i + 1);
    }

    // midi connections for the first and last plugin
    rack->addConnection ({}, 0, plugins.getFirst()->itemID, 0);
    rack->addConnection (plugins.getLast()->itemID, 0, {}, 0);

    for (int i = 0; i < plugins.size() - 1; ++i)
    {
        auto fsrc = plugins[i];
        auto fdst = plugins[i + 1];

        yup::StringArray dstIns, srcOuts;
        fsrc->getChannelNames (nullptr, &srcOuts);
        fdst->getChannelNames (&dstIns, nullptr);

        rack->addConnection (fsrc->itemID, 0, fdst->itemID, 0);

        for (int j = 0; j < std::min (srcOuts.size(), dstIns.size()); ++j)
            rack->addConnection (fsrc->itemID, j + 1, fdst->itemID, j + 1);
    }

    return rack;
}

yup::StringArray RackType::getInputNames() const
{
    yup::StringArray s;

    for (const auto& v : state)
        if (v.hasType (IDs::INPUT))
            s.add (v.getProperty (IDs::name));

    return s;
}

yup::StringArray RackType::getOutputNames() const
{
    yup::StringArray s;

    for (const auto& v : state)
        if (v.hasType (IDs::OUTPUT))
            s.add (v.getProperty (IDs::name));

    return s;
}

yup::Array<Plugin*> RackType::getPlugins() const
{
    yup::Array<Plugin*> list;

    for (auto i : pluginList->objects)
        if (i->plugin != nullptr)
            list.add (i->plugin.get());

    return list;
}

bool RackType::isPluginAllowed (const Plugin::Ptr& p)
{
    return p != nullptr && p->canBeAddedToRack();
}

bool RackType::addPlugin (const Plugin::Ptr& p, yup::Point<float> pos, bool canAutoConnect)
{
    if (! isPluginAllowed (p))
        return false;

    if (! getPlugins().contains (p.get()))
    {
        edit.getTransport().stopIfRecording();

        bool autoConnect = canAutoConnect && pluginList->objects.isEmpty();

        p->removeFromParent();

        auto v = createValueTree (IDs::PLUGININSTANCE,
                                  IDs::x, yup::jlimit (0.0f, 1.0f, pos.x),
                                  IDs::y, yup::jlimit (0.0f, 1.0f, pos.y));
        v.addChild (p->state, -1, getUndoManager());

        state.addChild (v, -1, getUndoManager());

        if (autoConnect)
        {
            yup::StringArray ins, outs;
            p->getChannelNames (&ins, &outs);

            while (outs.size() > getOutputNames().size() - 1)
                if (addOutput (getOutputNames().size(), TRANS("Output") + " " + yup::String (getOutputNames().size())) == -1)
                    break;

            for (int i = 0; i < ins.size(); ++i)   addConnection ({}, i + 1, p->itemID, i + 1);
            for (int i = 0; i < outs.size(); ++i)  addConnection (p->itemID, i + 1, {}, i + 1);

            // midi connections
            addConnection ({}, 0, p->itemID, 0);
            addConnection (p->itemID, 0, {}, 0);
        }

        return true;
    }

    return false;
}

yup::Point<float> RackType::getPluginPosition (const Plugin::Ptr& p) const
{
    for (auto info : pluginList->objects)
        if (info->plugin == p)
            return { info->state[IDs::x],
                     info->state[IDs::y] };

    return {};
}

yup::Point<float> RackType::getPluginPosition (int index) const
{
    if (auto info = pluginList->objects[index])
        return { info->state[IDs::x],
                 info->state[IDs::y] };

    return {};
}

void RackType::setPluginPosition (int index, yup::Point<float> pos)
{
    if (auto info = pluginList->objects[index])
    {
        info->state.setProperty (IDs::x, yup::jlimit (0.0f, 1.0f, pos.x), getUndoManager());
        info->state.setProperty (IDs::y, yup::jlimit (0.0f, 1.0f, pos.y), getUndoManager());
    }
}

void RackType::flushStateToValueTree()
{
    for (auto& plugin : getPlugins())
        plugin->flushPluginStateToValueTree();
}

//==============================================================================
yup::Array<EditItemID> RackType::getPluginsWhichTakeInputFrom (EditItemID sourceId) const
{
    yup::Array<EditItemID> results;

    if (sourceId.isValid())
        for (auto rc : connectionList->objects)
            if (rc->sourceID == sourceId && rc->destID.get().isValid())
                results.addIfNotAlreadyThere (rc->destID);

    return results;
}

bool RackType::arePluginsConnectedIndirectly (EditItemID src, EditItemID dest, int depth) const
{
    if (depth < 100) // to avoid loops
    {
        auto dests = getPluginsWhichTakeInputFrom (src);

        for (auto& d : dests)
            if (d == dest)
                return true;

        for (auto& d : dests)
            if (arePluginsConnectedIndirectly (d, dest, depth + 1))
                return true;
    }

    return false;
}

bool RackType::isConnectionLegal (EditItemID source, int sourcePin,
                                  EditItemID dest, int destPin) const
{
    if (! source.isValid())
    {
        if (sourcePin >= getInputNames().size())
            return false;
    }
    else
    {
        yup::StringArray ins, outs;

        if (auto p = edit.getPluginCache().getPluginFor (source))
            p->getChannelNames (&ins, &outs);

        if (sourcePin > outs.size())
            return false;
    }

    if (! dest.isValid())
    {
        if (destPin >= getOutputNames().size())
            return false;
    }
    else
    {
        yup::StringArray ins, outs;

        if (auto p = edit.getPluginCache().getPluginFor (dest))
            p->getChannelNames (&ins, &outs);
        else if (auto m = findModifierForID (getModifierList(), dest))
            ins = m->getAudioInputNames();

        if (destPin > ins.size())
            return false;
    }

    if (! (source.isValid() || dest.isValid()))
        return true;

    if (source == dest)
        return false;

    return ! arePluginsConnectedIndirectly (dest, source);
}

bool RackType::addConnection (EditItemID srcId, int sourcePin,
                              EditItemID dstId, int destPin)
{
    if (! isConnectionLegal (srcId, sourcePin, dstId, destPin))
        return false;

    for (auto rc : connectionList->objects)
        if (rc->destID == dstId && rc->destPin == destPin
              && rc->sourceID == srcId && rc->sourcePin == sourcePin)
            return false;

    auto v = createValueTree (IDs::CONNECTION,
                              IDs::src, srcId,
                              IDs::dst, dstId,
                              IDs::srcPin, sourcePin,
                              IDs::dstPin, destPin);

    state.addChild (v, -1, getUndoManager());
    return true;
}

bool RackType::removeConnection (EditItemID srcId, int sourcePin,
                                 EditItemID dstId, int destPin)
{
    TRACKTION_ASSERT_MESSAGE_THREAD

    for (int i = connectionList->objects.size(); --i >= 0;)
    {
        if (auto rc = connectionList->objects[i])
        {
            if (rc->destID == dstId && rc->sourceID == srcId
                 && rc->destPin == destPin && rc->sourcePin == sourcePin)
            {
                state.removeChild (rc->state, getUndoManager());
                return true;
            }
        }
    }

    return false;
}

void RackType::checkConnections()
{
    if (! edit.isLoading())
        TRACKTION_ASSERT_MESSAGE_THREAD

    for (int i = connectionList->objects.size(); --i >= 0;)
    {
        if (auto rc = connectionList->objects[i])
        {
            // Check for connections going to no longer available rack IO pins
            if (((rc->sourcePin < 0 || rc->sourcePin >= getInputNames().size()) && rc->sourceID->isInvalid())
                  || ((rc->destPin < 0 || rc->destPin >= getOutputNames().size()) && rc->destID->isInvalid()))
            {
                state.removeChild (rc->state, getUndoManager());
                continue;
            }

            // Avoid stripping connections when the Edit hasn't loaded the plugins as the channels will be 0
            if (! edit.shouldLoadPlugins())
                continue;

            // Check for connections going to no longer available plugin IO pins
            if (auto ep = dynamic_cast<ExternalPlugin*> (getPluginForID (rc->sourceID)))
            {
                if (auto pi = ep->getAudioPluginInstance())
                {
                    if (rc->sourcePin < 0 || rc->sourcePin > pi->getTotalNumOutputChannels())
                    {
                        state.removeChild (rc->state, getUndoManager());
                        continue;
                    }
                }
            }

            if (auto ep = dynamic_cast<ExternalPlugin*> (getPluginForID (rc->destID)))
            {
                if (auto pi = ep->getAudioPluginInstance())
                {
                    if (rc->destPin < 0 || rc->destPin > pi->getTotalNumInputChannels())
                    {
                        state.removeChild (rc->state, getUndoManager());
                        continue;
                    }
                }
            }
        }
    }
}

yup::Array<const RackConnection*> RackType::getConnections() const noexcept
{
    yup::Array<const RackConnection*> list;

    for (auto rc : connectionList->objects)
        list.add (rc);

    return list;
}

//==============================================================================
static bool findModifierWithID (yup::ValueTree& modifiers, EditItemID itemID)
{
    for (auto m : modifiers)
        if (EditItemID::fromID (m) == itemID)
            return true;

    return false;
}

static bool findPluginOrModifierWithID (yup::ValueTree& rack, EditItemID itemID)
{
    for (int i = rack.getNumChildren(); --i >= 0;)
    {
        auto c = rack.getChild (i);

        if (c.hasType (IDs::PLUGININSTANCE))
            if (EditItemID::fromID (c.getChildWithName (IDs::PLUGIN)) == itemID)
                return true;

        if (c.hasType (IDs::MODIFIERS))
            if (findModifierWithID (c, itemID))
                return true;
    }

    return false;
}

static int countNumConnections (yup::ValueTree& rack, const yup::Identifier& type)
{
    int count = 0;

    for (int i = rack.getNumChildren(); --i >= 0;)
        if (rack.getChild (i).hasType (type))
            ++count;

    return count;
}

static bool connectionIsValid (yup::ValueTree& rack, EditItemID srcID,
                               int srcPin, EditItemID dstID, int dstPin)
{
    if (srcID.isInvalid())
    {
        if (srcPin < 0 || srcPin >= countNumConnections (rack, IDs::INPUT))
            return false;
    }
    else if (! findPluginOrModifierWithID (rack, srcID))
    {
        return false;
    }

    if (dstID.isInvalid())
    {
        if (dstPin < 0 || dstPin >= countNumConnections (rack, IDs::OUTPUT))
            return false;
    }
    else if (! findPluginOrModifierWithID (rack, dstID))
    {
        return false;
    }

    return true;
}

void RackType::removeBrokenConnections (yup::ValueTree& rack, juce::UndoManager* um)
{
    if (um && um->isPerformingUndoRedo())
        return;

    for (int i = rack.getNumChildren(); --i >= 0;)
    {
        auto c = rack.getChild (i);

        if (c.hasType (IDs::CONNECTION)
             && ! connectionIsValid (rack,
                                     EditItemID::fromProperty (c, IDs::src), c[IDs::srcPin],
                                     EditItemID::fromProperty (c, IDs::dst), c[IDs::dstPin]))
            rack.removeChild (i, um);
    }
}

void RackType::createInstanceForSideChain (Track& at, const yup::BigInteger& channelMask,
                                           EditItemID pluginID, int pinIndex)
{
    const bool connectLeft  = channelMask[0];
    const bool connectRight = channelMask[1];
    auto& pl = at.pluginList;
    RackInstance* rack = nullptr;

    for (auto p : pl)
        if (auto rf = dynamic_cast<RackInstance*> (p))
            if (rf->type.get() == this)
                rack = rf;

    if (rack == nullptr)
    {
        if (auto p = edit.getPluginCache().getOrCreatePluginFor (RackInstance::create (*this)))
        {
            pl.insertPlugin (p, 0, nullptr);
            rack = dynamic_cast<RackInstance*> (p.get());
        }
    }

    if (rack != nullptr)
    {
        auto inputs = getInputNames();
        const yup::String leftName ("sidechain input (left)");
        const yup::String rightName ("sidechain input (right)");
        auto noneName = rack->getNoPinName();

        if (connectLeft)
        {
            int srcPinIndex = inputs.indexOf (leftName);

            if (srcPinIndex == -1)
                srcPinIndex = addInput (-1, leftName);

            rack->setInputMappingByName (0, leftName);
            rack->setOutputMappingByName (0, noneName);

            if (! connectRight)
            {
                rack->setInputMappingByName (1, noneName);
                rack->setOutputMappingByName (1, noneName);
            }

            if (srcPinIndex >= 0)
                addConnection ({}, srcPinIndex, pluginID, pinIndex);
        }

        if (connectRight)
        {
            int srcPinIndex = inputs.indexOf (rightName);

            if (srcPinIndex == -1)
                srcPinIndex = addInput (-1, rightName);

            rack->setInputMappingByName (1, rightName);
            rack->setOutputMappingByName (1, noneName);

            if (! connectLeft)
            {
                rack->setInputMappingByName (0, noneName);
                rack->setOutputMappingByName (0, noneName);
            }

            if (srcPinIndex != -1)
                addConnection ({}, srcPinIndex, pluginID,
                               connectLeft ? pinIndex + 1 : pinIndex);
        }

        rack->wetGain->setParameter (0.0f, yup::dontSendNotification);
        rack->dryGain->setParameter (1.0f, yup::dontSendNotification);

        SelectionManager::refreshAllPropertyPanelsShowing (*rack);
    }
    else
    {
        edit.engine.getUIBehaviour().showWarningAlert (TRANS("Unable to create side-chain"),
                                                       TRANS("Unable to create rack on source track"));
    }
}

//==============================================================================
yup::String RackType::getSelectableDescription()
{
    return TRANS("Plugin Rack");
}

//==============================================================================
static int findIndexOfNthInstanceOf (yup::ValueTree& parent, const yup::Identifier& type, int index)
{
    int count = 0;

    for (int i = 0; i < parent.getNumChildren(); ++i)
    {
        auto v = parent.getChild (i);

        if (v.hasType (type))
            if (count++ == index)
                return i;
    }

    return -1;
}

int RackType::addInput (int index, const yup::String& name)
{
    int numNames = getInputNames().size();

    if (numNames <= maxRackAudioChans)
    {
        if (index >= 0)
        {
            for (auto conn : connectionList->objects)
                if (conn->sourceID->isInvalid() && conn->sourcePin >= index)
                    conn->sourcePin = conn->sourcePin + 1;
        }

        state.addChild (createInOrOut (IDs::INPUT, name),
                        index < 0 ? findIndexOfNthInstanceOf (state, IDs::INPUT, numNames - 1) + 1
                                  : findIndexOfNthInstanceOf (state, IDs::INPUT, index),
                        getUndoManager());

        if (index < 0)
            index = numNames;

        jassert (getInputNames()[index] == name);
        return index;
    }

    return -1;
}

int RackType::addOutput (int index, const yup::String& name)
{
    int numNames = getOutputNames().size();

    if (numNames <= maxRackAudioChans)
    {
        if (index >= 0)
        {
            for (auto conn : connectionList->objects)
                if (conn->destID->isInvalid() && conn->destPin >= index)
                    conn->destPin = conn->destPin + 1;
        }

        state.addChild (createInOrOut (IDs::OUTPUT, name),
                        index < 0 ? findIndexOfNthInstanceOf (state, IDs::OUTPUT, numNames - 1) + 1
                                  : findIndexOfNthInstanceOf (state, IDs::OUTPUT, index),
                        getUndoManager());

        if (index < 0)
            index = numNames;

        jassert (getOutputNames()[index] == name);
        return index;
    }

    return -1;
}

void RackType::removeInput (int index)
{
    TRACKTION_ASSERT_MESSAGE_THREAD
    int toRemove = findIndexOfNthInstanceOf (state, IDs::INPUT, index);

    if (toRemove >= 0)
    {
        for (auto conn : connectionList->objects)
            if (conn->sourceID->isInvalid() && conn->sourcePin > index)
                conn->sourcePin = conn->sourcePin - 1;

        state.removeChild (toRemove, getUndoManager());
        checkConnections();
    }
}

void RackType::removeOutput (int index)
{
    TRACKTION_ASSERT_MESSAGE_THREAD
    int toRemove = findIndexOfNthInstanceOf (state, IDs::OUTPUT, index);

    if (toRemove >= 0)
    {
        for (auto conn : connectionList->objects)
            if (conn->destID->isInvalid() && conn->destPin > index)
                conn->destPin = conn->destPin - 1;

        state.removeChild (toRemove, getUndoManager());
        checkConnections();
    }
}

static yup::String checkChannelName (const yup::String& name)
{
    return name.trim().isEmpty() ? TRANS("Unnamed") : name;
}

void RackType::renameInput (int index, const yup::String& name)
{
    int toRename = findIndexOfNthInstanceOf (state, IDs::INPUT, index);

    if (toRename >= 0)
        state.getChild (toRename).setProperty (IDs::name, checkChannelName (name), getUndoManager());
}

void RackType::renameOutput (int index, const yup::String& name)
{
    int toRename = findIndexOfNthInstanceOf (state, IDs::OUTPUT, index);

    if (toRename >= 0)
        state.getChild (toRename).setProperty (IDs::name, checkChannelName (name), getUndoManager());
}

//==============================================================================
juce::UndoManager* RackType::getUndoManager() const
{
    return &edit.getUndoManager();
}

//==============================================================================
void RackType::registerInstance (RackInstance* instance, const PluginInitialisationInfo&)
{
    CRASH_TRACER
    activeRackInstances.addIfNotAlreadyThere (instance);
}

void RackType::deregisterInstance (RackInstance* instance)
{
    activeRackInstances.removeAllInstancesOf (instance);
}

void RackType::updateAutomatableParamPositions (TimePosition time)
{
    for (auto f : getPlugins())
        f->updateAutomatableParamPosition (time);

    for (auto m : getModifierList().getModifiers())
        m->updateAutomatableParamPosition (time);
}

//==============================================================================
RackType::Ptr RackType::findRackTypeContaining (const Plugin& plugin)
{
    for (auto p : getAllPlugins (plugin.edit, false))
        if (auto rack = dynamic_cast<RackInstance*> (p))
            if (rack->type != nullptr)
                if (rack->type->getPluginForID (plugin.itemID))
                    return rack->type;

    return {};
}

//==============================================================================
static yup::StringArray stripAndPrependChannels (yup::StringArray names)
{
    names.remove (0);
    int i = 0;

    for (auto&& n : names)
        n = yup::String (++i) + ". " + n;

    return names;
}

yup::StringArray RackType::getAudioInputNamesWithChannels() const
{
    return stripAndPrependChannels (getInputNames());
}

yup::StringArray RackType::getAudioOutputNamesWithChannels() const
{
    return stripAndPrependChannels (getOutputNames());
}

//==============================================================================
Plugin* RackType::getPluginForID (EditItemID pluginID)
{
    if (pluginID.isInvalid())
        return {};

    for (auto p : pluginList->objects)
        if (p->plugin != nullptr && p->plugin->itemID == pluginID)
            return p->plugin.get();

    return {};
}

void RackType::hideWindowForShutdown()
{
    for (auto af : getPlugins())
        if (auto ep = dynamic_cast<ExternalPlugin*> (af))
            ep->hideWindowForShutdown();

    for (auto ws : getWindowStates())
        ws->hideWindowForShutdown();
}

bool RackType::pasteClipboard()
{
    auto sm = edit.engine.getUIBehaviour().getSelectionManagerForRack (*this);

    if (sm == nullptr)
        return false;

    SelectableList pastedItems;

    if (auto plugins = Clipboard::getInstance()->getContentWithType<Clipboard::Plugins>())
    {
        for (auto& pluginState : plugins->plugins)
        {
            auto stateCopy = pluginState.createCopy();
            EditItemID::remapIDs (stateCopy, nullptr, edit);

            if (auto newPlugin = edit.getPluginCache().getOrCreatePluginFor (stateCopy))
            {
                auto selectedPlugin = sm->getFirstItemOfType<Plugin>();
                RackType::Ptr selectedRack (sm->getFirstItemOfType<RackType>());

                if (selectedPlugin != nullptr || selectedRack != nullptr)
                {
                    if (selectedRack == nullptr)
                        selectedRack = edit.getRackList().findRackContaining (*selectedPlugin);

                    if (selectedRack != nullptr)
                    {
                        yup::Point<float> pos (0.1f, 0.1f);

                        if (selectedPlugin != nullptr && selectedRack.get() == this)
                            pos = getPluginPosition (*selectedPlugin).translated (0.1f, 0.1f);

                        for (auto af : getPlugins())
                            if (pos == getPluginPosition (*af))
                                pos = pos.translated (0.1f, 0.1f);

                        selectedRack->addPlugin (newPlugin, pos, false);
                        pastedItems.add (newPlugin.get());
                    }
                }
            }
        }
    }

    sm->select (pastedItems);

    return pastedItems.isNotEmpty();
}

//==============================================================================
ModifierList& RackType::getModifierList() const noexcept
{
    return *modifierList;
}

//==============================================================================
struct RackTypeList::ValueTreeList  : public ValueTreeObjectList<RackType>
{
    ValueTreeList (RackTypeList& l, const yup::ValueTree& parentTree)
        : ValueTreeObjectList<RackType> (parentTree), list (l)
    {
        // NB: rebuildObjects() is called after construction so that the edit has a valid
        // list while they're being created
    }

    ~ValueTreeList() override
    {
        freeObjects();
    }

    bool isSuitableType (const yup::ValueTree& v) const override
    {
        return v.hasType (IDs::RACK);
    }

    RackType* createNewObject (const yup::ValueTree& v) override
    {
        auto t = new RackType (list.edit, v);
        t->incReferenceCount();
        return t;
    }

    void deleteObject (RackType* t) override
    {
        jassert (t != nullptr);
        t->decReferenceCount();
    }

    void newObjectAdded (RackType*) override                                            { sendChange(); }
    void objectRemoved (RackType*) override                                             { sendChange(); }
    void objectOrderChanged() override                                                  { sendChange(); }
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override  { sendChange(); }

    void sendChange()
    {
        // XXX
    }

    RackTypeList& list;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreeList)
};

//==============================================================================
RackTypeList::RackTypeList (Edit& ed) : edit (ed)
{
}

void RackTypeList::initialise (const yup::ValueTree& v)
{
    CRASH_TRACER

    state = v;
    jassert (state.hasType (IDs::RACKS));

    list = std::make_unique<ValueTreeList> (*this, v);
    list->rebuildObjects();
}

bool RackTypeList::isInitialised() const
{
    return list != nullptr;
}

RackTypeList::~RackTypeList()
{
    if (list)
        for (auto t : list->objects)
            t->hideWindowForShutdown();
}

const yup::Array<RackType*>& RackTypeList::getTypes() const noexcept
{
    assert (isInitialised());
    return list->objects;
}

int RackTypeList::size() const
{
    return list->objects.size();
}

RackType::Ptr RackTypeList::getRackType (int index) const
{
    return list->objects[index];
}

RackType::Ptr RackTypeList::getRackTypeForID (EditItemID rackID) const
{
    for (auto r : list->objects)
        if (r->itemID == rackID)
            return *r;

    return {};
}

RackType::Ptr RackTypeList::findRackContaining (Plugin& p) const
{
    for (auto r : list->objects)
        if (r->getPlugins().contains (&p))
            return *r;

    return {};
}

void RackTypeList::removeRackType (const RackType::Ptr& type)
{
    if (list->objects.contains (type.get()))
    {
        auto allTracks = getAllTracks (edit);

        for (auto f : getAllPlugins (edit, false))
            if (auto rf = dynamic_cast<RackInstance*> (f))
                if (rf->type == type)
                    rf->deleteFromParent();

        // Remove any Macros or Modifiers that might be assigned
        if (auto mpl = type->getMacroParameterList())
            mpl->hideMacroParametersFromTracks();

        for (auto macro : type->getMacroParameters())
            for (auto param : getAllParametersBeingModifiedBy (edit, *macro))
                param->removeModifier (*macro);

        for (auto modifier : type->getModifierList().getModifiers())
        {
            for (auto t : allTracks)
                t->hideAutomatableParametersForSource (modifier->itemID);

            for (auto param : getAllParametersBeingModifiedBy (edit, *modifier))
                param->removeModifier (*modifier);
        }

        type->hideWindowForShutdown();
        state.removeChild (type->state, &edit.getUndoManager());
    }
}

RackType::Ptr RackTypeList::addNewRack()
{
    auto newID = edit.createNewItemID();

    yup::ValueTree v (IDs::RACK);
    newID.writeID (v, nullptr);
    state.addChild (v, -1, &edit.getUndoManager());

    auto type = getRackTypeForID (newID);
    jassert (type != nullptr);

    if (edit.engine.getEngineBehaviour().arePluginsRemappedWhenTempoChanges())
        type->getMacroParameterListForWriting().remapOnTempoChange = true;

    return type;
}

RackType::Ptr RackTypeList::addRackTypeFrom (const yup::ValueTree& rackType)
{
    if (! rackType.hasType (IDs::RACK))
        return {};

    auto typeID = EditItemID::fromID (rackType);

    if (typeID.isInvalid())
        return {};

    auto type = getRackTypeForID (typeID);

    if (type == nullptr)
    {
        if (rackType.isValid())
        {
            state.addChild (rackType.createCopy(), -1, &edit.getUndoManager());

            type = getRackTypeForID (typeID);
            jassert (type != nullptr);

            if (edit.engine.getEngineBehaviour().arePluginsRemappedWhenTempoChanges())
                type->getMacroParameterListForWriting().remapOnTempoChange = true;
        }
    }

    return type;
}

RackType::Ptr RackTypeList::duplicateRack (RackType& rackType)
{
    rackType.flushStateToValueTree();
    auto newState = rackType.state.createCopy();
    EditItemID::remapIDs (newState, nullptr, edit);
    return addRackTypeFrom (newState);
}

RackType::Ptr RackTypeList::duplicateRack (EditItemID rackID)
{
    if (auto source = getRackTypeForID (rackID))
        return duplicateRack (*source);

    return {};
}

void RackTypeList::importRackFiles (const yup::Array<yup::File>& files)
{
    int oldNumRacks = size();

    for (auto& f : files)
        if (auto xml = yup::parseXML (f))
            addRackTypeFrom (yup::ValueTree::fromXml (*xml));

    if (oldNumRacks < size())
        edit.engine.getUIBehaviour().showWarningMessage (TRANS("Rack types added!"));
}

//==============================================================================
void RackType::triggerUpdate()
{
    CRASH_TRACER

    if (edit.isLoading())
        return;

    edit.restartPlayback();
}

void RackType::updateRenderContext()
{
    if (edit.isLoading())
        TRACKTION_ASSERT_MESSAGE_THREAD
}

//==============================================================================
void RackType::valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&)
{
    triggerUpdate();
}

void RackType::valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int)
{
    triggerUpdate();
}

void RackType::valueTreeChildOrderChanged (yup::ValueTree&, int, int)   {}
void RackType::valueTreeRedirected (yup::ValueTree&)                    { triggerUpdate(); }

void RackType::valueTreeParentChanged (yup::ValueTree&)
{
    if (! state.getParent().isValid())
        hideWindowForShutdown();

    triggerUpdate();
}

void RackType::valueTreePropertyChanged (yup::ValueTree& v, const yup::Identifier& ident)
{
    if (v.hasType (IDs::PLUGININSTANCE) || v.hasType (IDs::CONNECTION))
        if (ident != IDs::x && ident != IDs::y && ident != IDs::windowPos)
            triggerUpdate();

    if (v == state && ident == IDs::name)
    {
        rackName.forceUpdateOfCachedValue();

        for (auto af : getAllPlugins (edit, false))
            if (auto rf = dynamic_cast<RackInstance*> (af))
                if (rf->type.get() == this)
                    rf->changed();

        changed();
    }

    if (v == state && ident == IDs::id)
        jassertfalse;
}

//==============================================================================
RackType::WindowState::WindowState (RackType& r, yup::ValueTree windowStateTree)
    : PluginWindowState (r.edit), rack (r), state (std::move (windowStateTree))
{}

} // namespace tracktion::inline engine
