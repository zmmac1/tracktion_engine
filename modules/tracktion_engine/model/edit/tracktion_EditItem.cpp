/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

EditItem::EditItem (EditItemID id, Edit& ed)
    : edit (ed), itemID (id)
{
}

EditItem::EditItem (Edit& ed, const yup::ValueTree& v)
    : edit (ed), itemID (EditItemID::readOrCreateNewID (ed, v))
{
}


//==============================================================================
EditItemID EditItemID::fromVar (const yup::var& v)
{
    return fromRawID ((uint64_t) static_cast<yup::int64> (v));
}

EditItemID EditItemID::fromString (const yup::String& s)
{
    return fromRawID ((uint64_t) s.getLargeIntValue());
}

EditItemID EditItemID::fromID (const yup::ValueTree& v)
{
    return EditItemID::fromVar (v.getProperty (IDs::id));
}

void EditItemID::writeID (yup::ValueTree& v, juce::UndoManager* um) const
{
    v.setProperty (IDs::id, *this, um);
}

EditItemID EditItemID::fromRawID (uint64_t raw)
{
    EditItemID i;
    i.itemID = raw;
    return i;
}

static inline bool isSorted (const std::vector<EditItemID>& ids)
{
    uint64_t last = 0;

    for (auto id : ids)
    {
        auto i = id.getRawID();

        if (i < last)
            return false;

        last = i;
    }

    return true;
}

EditItemID EditItemID::findFirstIDNotIn (const std::vector<EditItemID>& existingIDs, uint64_t rawID)
{
    jassert (isSorted (existingIDs)); // the vector must be sorted

    for (auto existing : existingIDs)
    {
        auto i = existing.getRawID();

        if (i > rawID)
            break;

        ++rawID;
    }

    return fromRawID (rawID);
}

EditItemID EditItemID::fromProperty (const yup::ValueTree& v, const yup::Identifier& prop)
{
    return fromVar (v.getProperty (prop));
}

EditItemID EditItemID::fromXML (const yup::XmlElement& xml, const char* attributeName)
{
    return fromVar (xml.getStringAttribute (attributeName));
}

EditItemID EditItemID::fromXML (const yup::XmlElement& xml, const yup::Identifier& attributeName)
{
    return fromVar (xml.getStringAttribute (attributeName));
}

void EditItemID::setProperty (yup::ValueTree& v, const yup::Identifier& prop, juce::UndoManager* um) const
{
    v.setProperty (prop, toVar(), um);
}

void EditItemID::setXML (yup::XmlElement& xml, const yup::Identifier& attributeName) const
{
    xml.setAttribute (attributeName, toString());
}

void EditItemID::setXML (yup::XmlElement& xml, const char* attributeName) const
{
    xml.setAttribute (attributeName, toString());
}

yup::var EditItemID::toVar() const
{
    return yup::var (static_cast<yup::int64> (itemID));
}

yup::String EditItemID::toString() const
{
    return toVar().toString();
}

EditItemID EditItemID::readOrCreateNewID (Edit& edit, const yup::ValueTree& v)
{
    auto i = fromID (v);

    if (i.isValid())
        return i;

    i = edit.createNewItemID();
    yup::ValueTree localTree (v);
    i.writeID (localTree, nullptr);
    return i;
}

yup::Array<EditItemID> EditItemID::parseStringList (const yup::String& list)
{
    yup::Array<EditItemID> trackIDs;
    auto items = yup::StringArray::fromTokens (list, ",|", {});
    trackIDs.ensureStorageAllocated (items.size());

    for (auto& s : items)
        trackIDs.add (fromVar (s));

    return trackIDs;
}

yup::String EditItemID::listToString (const yup::Array<EditItemID>& items)
{
    if (items.isEmpty())
        return {};

    if (items.size() == 1)
        return items.getFirst().toString();

    yup::StringArray ids;
    ids.ensureStorageAllocated (items.size());

    for (auto& i : items)
        ids.add (i.toString());

    return ids.joinIntoString (",");
}

//==============================================================================
struct IDRemapping
{
    static bool isIDDeclaration (yup::StringRef att)
    {
        return att == IDs::id || att == IDs::groupID;
    }

    static bool isIDReference (const yup::Identifier& parentType, yup::StringRef att)
    {
        for (auto p : { IDs::currentAutoParamPluginID, IDs::currentAutoParamTag,
                        IDs::targetID, IDs::sourceTrack, IDs::src, IDs::dst,
                        IDs::pluginID, IDs::rackType, IDs::paramID })
            if (p == att)
                return true;

        if (att == IDs::source && ! Clip::isClipState (parentType))
            return true;

        if (att == IDs::track && parentType == IDs::COMPSECTION)
            return true;

        return false;
    }

    static bool isIDRefOrDecl (const yup::XmlElement& xml, yup::StringRef att)
    {
        return isIDDeclaration (att) || isIDReference (xml.getTagName(), att);
    }

    static bool isIDRefOrDecl (const yup::ValueTree& v, yup::StringRef att)
    {
        return isIDDeclaration (att) || isIDReference (v.getType(), att);
    }

    static bool isIDList (yup::StringRef att)
    {
        return att == IDs::hiddenClips || att == IDs::lockedClips
                || att == IDs::ghostTracks || att == IDs::renderTracks;
    }

    template <typename Visitor>
    static void visitAllIDDecls (const yup::XmlElement& xml, Visitor&& visitor)
    {
        for (int i = 0; i < xml.getNumAttributes(); ++i)
            if (isIDDeclaration (xml.getAttributeName (i)))
                visitor (xml.getAttributeValue (i));

        for (auto e : xml.getChildIterator())
            visitAllIDDecls (*e, visitor);
    }

    template <typename Visitor>
    static void visitAllIDDecls (const yup::ValueTree& v, Visitor&& visitor)
    {
        for (int i = 0; i < v.getNumProperties(); ++i)
        {
            auto propName = v.getPropertyName (i);

            if (isIDDeclaration (propName))
                visitor (v.getProperty (propName));
        }

        for (const auto& child : v)
            visitAllIDDecls (child, visitor);
    }

    using StringToIDMap = std::map<yup::String, EditItemID>;

    static yup::String remapIDList (const yup::String& list, const StringToIDMap& newIDsToApply)
    {
        yup::Array<EditItemID> newItemIDs;

        for (auto oldID : yup::StringArray::fromTokens (list, "|,", {}))
        {
            if (oldID.isNotEmpty())
            {
                auto newID = newIDsToApply.find (oldID);

                if (newID != newIDsToApply.end())
                    newItemIDs.add (newID->second);
            }
        }

        return EditItemID::listToString (newItemIDs);
    }

    static void applyNewIDs (yup::XmlElement& xml, const StringToIDMap& newIDsToApply)
    {
        for (int i = 0; i < xml.getNumAttributes(); ++i)
        {
            auto attName = xml.getAttributeName (i);

            if (isIDRefOrDecl (xml, attName))
            {
                auto oldID = xml.getAttributeValue (i);

                if (oldID.isNotEmpty())
                {
                    auto newID = newIDsToApply.find (oldID);

                    if (newID != newIDsToApply.end())
                        xml.setAttribute (attName, newID->second.toString());
                }
            }
            else if (isIDList (attName))
            {
                xml.setAttribute (attName, remapIDList (xml.getAttributeValue (i), newIDsToApply));
            }

            if (EditItemID::applyNewIDsToExternalXML != nullptr)
                EditItemID::applyNewIDsToExternalXML (xml, attName, newIDsToApply);
        }

        for (auto e : xml.getChildIterator())
            applyNewIDs (*e, newIDsToApply);
    }

    static void applyNewIDs (yup::ValueTree& v, const StringToIDMap& newIDsToApply, juce::UndoManager* um)
    {
        for (int i = 0; i < v.getNumProperties(); ++i)
        {
            auto propName = v.getPropertyName (i);

            if (isIDRefOrDecl (v, propName))
            {
                auto oldID = v.getProperty (propName).toString();

                if (oldID.isNotEmpty())
                {
                    auto newID = newIDsToApply.find (oldID);

                    if (newID != newIDsToApply.end())
                    {
                        v.setProperty (propName, newID->second, um);
                    }
                    else
                    {
                        DBG ("Dangling ID found: " << oldID);
                    }
                }
            }
            else if (isIDList (propName))
            {
                v.setProperty (propName, remapIDList (v.getProperty (propName), newIDsToApply), um);
            }

            if (EditItemID::applyNewIDsToExternalValueTree)
                EditItemID::applyNewIDsToExternalValueTree (v, propName, newIDsToApply, um);
        }

        for (auto child : v)
            applyNewIDs (child, newIDsToApply, um);
    }

    static void addItemsToReturnedMap (const StringToIDMap& newIDs, EditItemID::IDMap* result)
    {
        if (result != nullptr)
        {
            for (auto& i : newIDs)
            {
                auto itemID = EditItemID::fromString (i.first);

                if (itemID.isValid())
                    (*result)[itemID] = i.second;
                else
                    jassertfalse; // we're trying to get out a list of ID -> ID, but some
                                  // of the original IDs weren't valid. Could be that they were
                                  // legacy string IDs?
            }
        }
    }
};

std::vector<EditItemID> EditItemID::findAllIDs (const yup::XmlElement& xml)
{
    std::vector<EditItemID> ids;

    IDRemapping::visitAllIDDecls (xml, [&] (const yup::String& oldID)
    {
        auto i = EditItemID::fromString (oldID);

        if (i.isValid())
            ids.push_back (i);
    });

    return ids;
}

std::vector<EditItemID> EditItemID::findAllIDs (const yup::ValueTree& v)
{
    std::vector<EditItemID> ids;

    IDRemapping::visitAllIDDecls (v, [&] (const yup::var& oldID)
    {
        auto i = EditItemID::fromVar (oldID);

        if (i.isValid())
            ids.push_back (i);
    });

    return ids;
}

void EditItemID::remapIDs (yup::XmlElement& xml, std::function<EditItemID()> createNewID, IDMap* remappedIDs)
{
    IDRemapping::StringToIDMap newIDs;

    IDRemapping::visitAllIDDecls (xml, [&] (const yup::String& oldID)
    {
        if (oldID.isNotEmpty())
        {
            auto& remapped = newIDs[oldID];

            if (remapped.isInvalid())
            {
                remapped = createNewID();
                //DBG ("Remapping ID: " << oldID << "  " << remapped.toString());
            }
        }
    });

    IDRemapping::applyNewIDs (xml, newIDs);
    IDRemapping::addItemsToReturnedMap (newIDs, remappedIDs);
}

void EditItemID::remapIDs (yup::ValueTree& v, juce::UndoManager* um,
                           std::function<EditItemID()> createNewID, IDMap* remappedIDs)
{
    IDRemapping::StringToIDMap newIDs;

    IDRemapping::visitAllIDDecls (v, [&] (const yup::var& oldID)
    {
        auto oldIDString = oldID.toString();

        if (oldIDString.isNotEmpty())
        {
            auto& remapped = newIDs[oldIDString];

            if (remapped.isInvalid() && remappedIDs != nullptr && remappedIDs->contains (EditItemID::fromString (oldIDString)))
            {
                remapped = remappedIDs->at (EditItemID::fromString (oldIDString));
                //DBG ("Remapping ID: " << oldIDString << "  " << remapped.toString());
            }

            if (remapped.isInvalid())
            {
                remapped = createNewID();
                //DBG ("Remapping ID: " << oldIDString << "  " << remapped.toString());
            }
        }
    });

    IDRemapping::applyNewIDs (v, newIDs, um);
    IDRemapping::addItemsToReturnedMap (newIDs, remappedIDs);
}

struct MultipleNewIDGenerator
{
    MultipleNewIDGenerator (Edit& ed, EditItemID::IDMap* remappedIDs)  : edit (ed)
    {
        if (remappedIDs != nullptr)
        {
            alreadyUsed.reserve (alreadyUsed.size() + remappedIDs->size());

            for (auto& i : *remappedIDs)
                alreadyUsed.push_back (i.second);
        }
    }

    EditItemID operator()()
    {
        auto newID = edit.createNewItemID (alreadyUsed);
        alreadyUsed.push_back (newID);
        return newID;
    }

    Edit& edit;
    std::vector<EditItemID> alreadyUsed;
};

void EditItemID::remapIDs (yup::XmlElement& xml, Edit& ed, IDMap* remappedIDs)
{
    MultipleNewIDGenerator idCreator (ed, remappedIDs);
    remapIDs (xml, idCreator, remappedIDs);
}

void EditItemID::remapIDs (yup::ValueTree& v, juce::UndoManager* um, Edit& ed, IDMap* remappedIDs)
{
    MultipleNewIDGenerator idCreator (ed, remappedIDs);
    remapIDs (v, um, idCreator, remappedIDs);
}

//==============================================================================
std::function<void (yup::ValueTree&, const yup::Identifier&, const std::map<yup::String, EditItemID>&, juce::UndoManager*)> EditItemID::applyNewIDsToExternalValueTree;
std::function<void (yup::XmlElement&, const yup::String&, const std::map<yup::String, EditItemID>&)> EditItemID::applyNewIDsToExternalXML;

} // namespace tracktion::inline engine
