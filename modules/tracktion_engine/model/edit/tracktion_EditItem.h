/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

//==============================================================================
/**
    ID for objects of type EditElement - e.g. clips, tracks, plugins, etc
*/
struct EditItemID
{
    EditItemID() = default;
    EditItemID (const EditItemID&) = default;
    EditItemID& operator= (const EditItemID&) = default;

    bool isValid() const noexcept       { return itemID != 0; }
    bool isInvalid() const noexcept     { return itemID == 0; }

    static EditItemID fromID (const yup::ValueTree&);
    static EditItemID readOrCreateNewID (Edit&, const yup::ValueTree&);
    void writeID (yup::ValueTree&, juce::UndoManager*) const;

    static EditItemID fromVar (const yup::var&);
    static EditItemID fromString (const yup::String&);
    static EditItemID fromProperty (const yup::ValueTree&, const yup::Identifier&);
    static EditItemID fromXML (const yup::XmlElement&, const char* attributeName);
    static EditItemID fromXML (const yup::XmlElement&, const yup::Identifier&);

    static EditItemID findFirstIDNotIn (const std::vector<EditItemID>& existingIDsSorted,
                                        uint64_t startFrom = 1001);

    void setProperty (yup::ValueTree&, const yup::Identifier&, juce::UndoManager*) const;
    void setXML (yup::XmlElement&, const yup::Identifier& attributeName) const;
    void setXML (yup::XmlElement&, const char* attributeName) const;

    operator yup::var() const                          { return toVar(); }
    yup::var toVar() const;
    yup::String toString() const;

    uint64_t getRawID() const noexcept                  { return itemID; }
    static EditItemID fromRawID (uint64_t);

    bool operator== (EditItemID other) const noexcept   { return itemID == other.itemID; }
    bool operator!= (EditItemID other) const noexcept   { return itemID != other.itemID; }
    bool operator<  (EditItemID other) const noexcept   { return itemID <  other.itemID; }

    static yup::Array<EditItemID> parseStringList (const yup::String& list);
    static yup::String listToString (const yup::Array<EditItemID>& list);

    //==============================================================================
    static std::vector<EditItemID> findAllIDs (const yup::XmlElement&);
    static std::vector<EditItemID> findAllIDs (const yup::ValueTree&);

    using IDMap = std::map<EditItemID, EditItemID>;

    static void remapIDs (yup::XmlElement&, std::function<EditItemID()> createNewID, IDMap* remappedIDs = nullptr);
    static void remapIDs (yup::XmlElement&, Edit&, IDMap* remappedIDs = nullptr);
    static void remapIDs (yup::ValueTree&, juce::UndoManager*, std::function<EditItemID()> createNewID, IDMap* remappedIDs = nullptr);
    static void remapIDs (yup::ValueTree&, juce::UndoManager*, Edit&, IDMap* remappedIDs = nullptr);

    //==============================================================================
    /** Callback that can be set in order to update any reassigned IDs in ValueTree client code. */
    static std::function<void (yup::ValueTree&, const yup::Identifier& /*propertyName*/,
                               const std::map<yup::String, EditItemID>&, juce::UndoManager*)> applyNewIDsToExternalValueTree;

    /** Callback that can be set in order to update any reassigned IDs in XML client code. */
    static std::function<void (yup::XmlElement&, const yup::String& /*propertyName*/,
                               const std::map<yup::String, EditItemID>&)> applyNewIDsToExternalXML;

private:
    uint64_t itemID = 0;
};

//==============================================================================
/**
    Base class for objects that live inside an edit - e.g. clips, tracks, plguins, etc
*/
class EditItem
{
public:
    EditItem (EditItemID, Edit&);
    EditItem (Edit&, const yup::ValueTree&);
    virtual ~EditItem() = default;

    //==============================================================================
    Edit& edit;

    /** Every EditItem has an ID which is unique within the edit. */
    const EditItemID itemID;

    virtual yup::String getName() const = 0;

private:
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditItem)
};


//==============================================================================
/** */
template<typename EditItemType>
struct EditItemCache
{
    EditItemCache() = default;
    ~EditItemCache() { jassert (knownEditItems.empty()); }

    EditItemType* findItem (EditItemID id) const
    {
        auto o = knownEditItems.find (id);

        if (o != knownEditItems.cend())
            return o->second;

        return {};
    }

    template<typename Visitor>
    void visitItems (Visitor&& visitor) const
    {
        for (auto iter : knownEditItems)
            std::forward<Visitor> (visitor)(iter.second);
    }

    void addItem (EditItemType& item)
    {
        jassert (knownEditItems.find (item.itemID) == knownEditItems.end());

        if (item.itemID.isValid())
            knownEditItems[item.itemID] = &item;
    }

    void removeItem (EditItemType& item)
    {
        if (item.itemID.isValid())
            knownEditItems.erase (item.itemID);
    }

private:
    std::map<EditItemID, EditItemType*> knownEditItems;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditItemCache)
};

} // namespace tracktion::inline engine

namespace yup
{
    template <>
    struct VariantConverter<tracktion::engine::EditItemID>
    {
        static tracktion::engine::EditItemID fromVar (const yup::var& v)   { return tracktion::engine::EditItemID::fromVar (v); }
        static yup::var toVar (const tracktion::engine::EditItemID& v)     { return yup::var ((yup::int64) v.getRawID()); }
    };
}

#ifndef DOXYGEN
namespace std
{
    template<>
    struct hash<tracktion::engine::EditItemID>
    {
        size_t operator() (const tracktion::engine::EditItemID& e) const noexcept   { return (size_t) e.getRawID(); }
    };
}
#endif
