/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

struct BankSet
{
    BankSet() noexcept {}
    BankSet (const yup::String& nm) noexcept : name (nm) {}

    BankSet (int high, int low, const yup::String& name_) noexcept
       : name (name_), hi (high), lo (low)
    {}

    yup::String name;
    yup::StringArray patches;
    yup::Array<int> patchNumbers;
    int hi = 0, lo = 0;
};

struct ProgramSet
{
    yup::String name;
    yup::OwnedArray<BankSet> banks;
};

inline std::unique_ptr<yup::XmlElement> exportProgramSet (ProgramSet& set)
{
    if (set.banks.isEmpty())
        return {};

    auto rootXml = std::make_unique<yup::XmlElement> ("ProgramSet");
    rootXml->setAttribute ("name", set.name);

    for (auto bankSet : set.banks)
    {
        auto bankXml = rootXml->createNewChildElement ("bank");
        bankXml->setAttribute ("name", bankSet->name);
        bankXml->setAttribute ("high", bankSet->hi);
        bankXml->setAttribute ("low", bankSet->lo);

        for (int j = 0; j < bankSet->patches.size(); ++j)
        {
            auto programXml = bankXml->createNewChildElement ("program");
            programXml->setAttribute ("name", bankSet->patches[j]);
            programXml->setAttribute ("number", bankSet->patchNumbers[j]);
        }
    }

    return rootXml;
}

inline BankSet* getBankSet (ProgramSet& set, const yup::String& name)
{
    for (int i = 0; i < set.banks.size(); ++i)
        if (set.banks.getUnchecked (i)->name == name)
            return set.banks.getUnchecked (i);

    return set.banks.add (new BankSet (name));
}

inline std::unique_ptr<yup::XmlElement> convertMidnamToXml (const yup::File& src)
{
    yup::String manufacturer;
    yup::StringArray models;

    auto rootXml = yup::parseXML (src);

    if (rootXml == nullptr)
    {
        // might be wacky corrupt file, search for xml
        yup::MemoryBlock mb;
        src.loadFileAsData (mb);

        for (size_t i = 0; i < mb.getSize(); ++i)
            if (mb[i] == 0)
                mb[i] = 32;

        auto str = mb.toString();

        int firstidx = str.indexOf("<");
        int lastidx  = str.lastIndexOf(">");

        if (firstidx != -1 && lastidx != -1)
            str = str.substring(firstidx, lastidx);

        rootXml = yup::parseXML (str);

        if (rootXml == nullptr)
            return {};
    }

    auto devXml = rootXml->getChildByName ("MasterDeviceNames");

    if (devXml == nullptr)
        return {};

    if (auto manufacturerXml = devXml->getChildByName ("Manufacturer"))
        manufacturer += manufacturerXml->getAllSubText();

    for (auto modelXml : devXml->getChildWithTagNameIterator ("Model"))
        models.add (modelXml->getAllSubText());

    ProgramSet programSet;
    programSet.name = manufacturer;

    for (int i = 0; i < models.size(); ++i)
        programSet.name += " " + models[i];

    programSet.name.trim();

    for (auto patchNameListXml : devXml->getChildWithTagNameIterator ("PatchNameList"))
    {
        auto name = patchNameListXml->getStringAttribute("Name");
        auto bankSet = getBankSet (programSet, name);

        for (auto patchXml : patchNameListXml->getChildWithTagNameIterator ("Patch"))
        {
            auto patchName = patchXml->getStringAttribute ("Name");

            if (patchName.isNotEmpty())
            {
                int num = patchXml->getIntAttribute("ProgramChange");

                if (! bankSet->patchNumbers.contains (num))
                {
                    bankSet->patches.add (patchName);
                    bankSet->patchNumbers.add (num);
                }
            }
        }
    }

    for (auto channelNameSetXml : devXml->getChildWithTagNameIterator ("ChannelNameSet"))
    {
        for (auto patchBankXml : channelNameSetXml->getChildWithTagNameIterator ("PatchBank"))
        {
            auto name = patchBankXml->getStringAttribute ("Name");
            auto bankSet = getBankSet (programSet, name);

            if (auto midiCommandsXml = patchBankXml->getChildByName ("MIDICommands"))
            {
                for (auto controlChangeXml : midiCommandsXml->getChildWithTagNameIterator ("ControlChange"))
                {
                    if (controlChangeXml->hasAttribute ("Control") && controlChangeXml->hasAttribute("Value"))
                    {
                        switch (controlChangeXml->getIntAttribute ("Control"))
                        {
                            case 0 : bankSet->hi = controlChangeXml->getIntAttribute ("Value"); break;
                            case 32: bankSet->lo = controlChangeXml->getIntAttribute ("Value"); break;
                        }
                    }
                }
            }

            if (auto patchNameListXml = patchBankXml->getChildByName ("PatchNameList"))
            {
                for (auto patchXml : patchNameListXml->getChildWithTagNameIterator ("Patch"))
                {
                    auto patchName = patchXml->getStringAttribute("Name");

                    if (patchName.isNotEmpty())
                    {
                        const int num = patchXml->getIntAttribute ("ProgramChange");

                        if (! bankSet->patchNumbers.contains (num))
                        {
                            bankSet->patches.add (patchName);
                            bankSet->patchNumbers.add (num);
                        }
                    }
                }
            }
        }
    }

    for (int i = programSet.banks.size(); --i >= 0;)
        if (auto b = programSet.banks.getUnchecked (i))
            if (b->patches.isEmpty())
                programSet.banks.remove (i);

    return exportProgramSet (programSet);
}

//==============================================================================
MidiProgramManager::MidiProgramSet::MidiProgramSet()
{
    for (int i = 0; i < 16; ++i)
    {
        midiBanks[i].id = i << 8;
        midiBanks[i].name = TRANS("Bank") + " " + yup::String (i + 1);
    }
}

yup::XmlElement* MidiProgramManager::MidiBank::createXml()
{
    auto n = new yup::XmlElement ("bank");
    n->setAttribute ("name", name);
    n->setAttribute ("low", id % 256);
    n->setAttribute ("high", id / 256);

    for (auto& p : programNames)
    {
        auto currentProgramXml = n->createNewChildElement ("program");
        currentProgramXml->setAttribute ("number", p.first);
        currentProgramXml->setAttribute ("name", p.second);
    }

    return n;
}

void MidiProgramManager::MidiBank::updateFromXml (const yup::XmlElement& n)
{
    name = n.getStringAttribute ("name");
    id = n.getIntAttribute ("high") * 256 + n.getIntAttribute ("low");

    for (auto programXml : n.getChildWithTagNameIterator ("program"))
    {
        programNames[programXml->getIntAttribute ("number")]
            = programXml->getStringAttribute ("name");
    }
}

//==============================================================================
yup::XmlElement* MidiProgramManager::MidiProgramSet::createXml()
{
    auto n = new yup::XmlElement ("ProgramSet");
    n->setAttribute ("name", name);
    n->setAttribute ("zeroBased", zeroBased);

    for (int i = 0; i < 16; ++i)
        n->addChildElement (midiBanks[i].createXml());

    return n;
}

void MidiProgramManager::MidiProgramSet::updateFromXml (const yup::XmlElement& n)
{
    name = n.getStringAttribute ("name");
    zeroBased = n.getBoolAttribute ("zeroBased", false);

    int idx = 0;

    for (auto currentChildXml : n.getChildIterator())
    {
        if (currentChildXml->hasTagName ("bank"))
            midiBanks[idx++].updateFromXml (*currentChildXml);

        if (idx >= yup::numElementsInArray (midiBanks))
            break;
    }
}

//==============================================================================
MidiProgramManager::MidiProgramManager (Engine& e)
    : engine (e)
{
    auto xml = e.getPropertyStorage().getXmlProperty (SettingID::midiProgramManager);

    if (xml != nullptr)
    {
        for (auto currentSetXml : xml->getChildIterator())
        {
            auto* currentSet = new MidiProgramSet();
            currentSet->updateFromXml (*currentSetXml);
            programSets.add (currentSet);
        }
    }

    if (xml == nullptr || ! xml->getBoolAttribute ("createdRootGroup", false))
    {
        auto* defaultSet = new MidiProgramSet();
        defaultSet->name = TRANS("Custom");
        programSets.add (defaultSet);

        saveAll();
    }
}

void MidiProgramManager::saveAll()
{
    yup::XmlElement xml ("MidiProgramManager");
    xml.setAttribute ("createdRootGroup", true);

    for (int i = 0; i < programSets.size(); ++i)
    {
        auto* currentSet = programSets[i];
        auto* currentSetXml = currentSet->createXml();
        xml.addChildElement (currentSetXml);
    }

    engine.getPropertyStorage().setXmlProperty (SettingID::midiProgramManager, xml);
}

yup::StringArray MidiProgramManager::getMidiProgramSetNames()
{
    yup::StringArray names;
    names.add (TRANS("General MIDI"));

    for (int i = 0; i < programSets.size(); ++i)
        names.add (programSets.getUnchecked (i)->name);

    return names;
}

yup::String MidiProgramManager::getProgramName (int set, int bank, int program)
{
    if (set == 0)
        return TRANS(yup::MidiMessage::getGMInstrumentName (program));

    if (auto p = programSets[set - 1])
    {
        if (bank >= 0 && bank < yup::numElementsInArray (p->midiBanks))
        {
            auto n = p->midiBanks[bank].programNames.find (program);

            if (n != p->midiBanks[bank].programNames.end())
                return n->second;
        }
    }

    return getDefaultName (bank, program, programSets[set - 1]->zeroBased);
}

yup::String MidiProgramManager::getBankName (int set, int bank)
{
    if (auto p = programSets[set - 1])
    {
        if (bank >= 0 && bank < yup::numElementsInArray (p->midiBanks))
        {
            auto name = p->midiBanks[bank].name;

            if (name.isNotEmpty())
                return name;
        }
    }

    return TRANS("Bank") + " " + yup::String (bank + 1);
}

int MidiProgramManager::getBankID (int set, int bank)
{
    if (auto p = programSets[set - 1])
        if (bank >= 0 && bank < yup::numElementsInArray (p->midiBanks))
            return p->midiBanks[bank].id;

    return bank;
}

void MidiProgramManager::setBankID (int set, int bank, int id)
{
    if (auto p = programSets[set - 1])
        if (bank >= 0 && bank < yup::numElementsInArray (p->midiBanks))
            p->midiBanks[bank].id = id;
}

bool MidiProgramManager::isZeroBased (int set)
{
    if (auto p = programSets[set - 1])
        return p->zeroBased;

    return false;
}

void MidiProgramManager::setZeroBased (int set, bool zeroBased)
{
    if (auto p = programSets[set - 1])
        p->zeroBased = zeroBased;
}

void MidiProgramManager::setProgramName (int set, int bank, int program, const yup::String& newName)
{
    if (auto p = programSets[set - 1])
        if (bank >= 0 && bank < yup::numElementsInArray (p->midiBanks))
            p->midiBanks[bank].programNames[program] = newName;
}

void MidiProgramManager::setBankName (int set, int bank, const yup::String& newName)
{
    if (auto p = programSets[set - 1])
        if (bank >= 0 && bank < yup::numElementsInArray (p->midiBanks))
            p->midiBanks[bank].name = newName;
}

void MidiProgramManager::clearProgramName (int set, int bank, int program)
{
    if (auto p = programSets[set - 1])
        if (bank >= 0 && bank < yup::numElementsInArray (p->midiBanks))
            p->midiBanks[bank].programNames.erase (program);
}

yup::String MidiProgramManager::getDefaultName (int, int program, bool zeroBased)
{
    return TRANS("Patch") + " "
            + yup::String (program % 128 + (zeroBased ? 1 : 0));
}

yup::String MidiProgramManager::getDefaultCustomName()
{
    for (int i = 0; i < programSets.size(); ++i)
        if (programSets.getUnchecked (i)->name == TRANS("Custom"))
            return TRANS("Custom");

    return TRANS("General MIDI");
}

int MidiProgramManager::getSetIndex (const yup::String& setName)
{
    for (int i = 0; i < programSets.size(); ++i)
        if (programSets.getUnchecked (i)->name == setName)
            return i + 1;

    return 0;
}

bool MidiProgramManager::doesSetExist (const yup::String& name) const noexcept
{
    if (name == TRANS("General MIDI"))
        return true;

    for (int i = 0; i < programSets.size(); ++i)
        if (programSets.getUnchecked (i)->name == name)
            return true;

    return false;
}

bool MidiProgramManager::canEditProgramSet (int set) const noexcept
{
    return set > 0;
}

bool MidiProgramManager::canDeleteProgramSet (int set) const noexcept
{
    return set > 0;
}

void MidiProgramManager::addNewSet (const yup::String& name)
{
    auto newSet = programSets.add (new MidiProgramSet());
    newSet->name = name;
    saveAll();
}

void MidiProgramManager::addNewSet (const yup::String& name, const yup::XmlElement& element)
{
    auto newSet = programSets.add (new MidiProgramSet());
    newSet->updateFromXml (element);
    newSet->name = name;
    saveAll();
}

void MidiProgramManager::deleteSet (int set)
{
    programSets.remove (set - 1);
    saveAll();
}

bool MidiProgramManager::importSet (int set, const yup::File& file)
{
    bool ok = false;

    if (auto xml = file.hasFileExtension("midnam") ? convertMidnamToXml (file)
                                                   : yup::parseXML (file))
    {
        if (auto s = programSets[set - 1])
        {
            auto oldName = s->name;
            s->updateFromXml (*xml);
            s->name = oldName;
            ok = true;
        }
    }

    saveAll();
    return ok;
}

bool MidiProgramManager::exportSet (int set, const yup::File& file)
{
    std::unique_ptr<yup::XmlElement> xml;

    if (auto programSet = programSets[set - 1])
        xml.reset (programSet->createXml());

    return xml != nullptr && xml->writeTo (file);
}

static yup::File getPatchNamesZipFile (Engine& engine)
{
    auto presetZipFile = engine.getTemporaryFileManager().getTempDirectory()
                            .getChildFile ("patchnames.zip");

    if (! presetZipFile.exists())
    {
        presetZipFile.getParentDirectory().createDirectory();
        presetZipFile.replaceWithData (TracktionBinaryData::patchnames_zip, TracktionBinaryData::patchnames_zipSize);
    }

    return presetZipFile;
}

yup::StringArray MidiProgramManager::getListOfPresets (Engine& engine)
{
    yup::StringArray presets;
    yup::ZipFile zf (getPatchNamesZipFile (engine));

    for (int i = 0; i < zf.getNumEntries(); ++i)
    {
        auto entry = zf.getEntry(i);
        auto lastDot = entry->filename.lastIndexOfChar ('.');

        if (lastDot >= 0)
            presets.add (entry->filename.substring (0, lastDot));
        else
            presets.add (entry->filename);
    }

    presets.sort (true);

    return presets;
}

yup::String MidiProgramManager::getPresetXml (yup::String presetName)
{
    yup::ZipFile zf (getPatchNamesZipFile (engine));

    for (int i = 0; i < zf.getNumEntries(); ++i)
    {
        auto entry = zf.getEntry(i);
        auto lastDot = entry->filename.lastIndexOfChar ('.');
        auto curName = (lastDot >= 0) ? entry->filename.substring (0, lastDot)
                                      : entry->filename;

        if (curName == presetName)
        {
            std::unique_ptr<yup::InputStream> is (zf.createStreamForEntry(i));

            if (is != nullptr)
                return is->readEntireStreamAsString();
        }
    }

    return {};
}

} // namespace tracktion::inline engine
