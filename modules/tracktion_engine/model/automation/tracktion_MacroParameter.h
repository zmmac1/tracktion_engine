/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/**
    A MacroParameter is an AutomatableParameter which is a collection of
    Mappings. A Mapping is a reference to an AutomatableParameter with a range and curve.
*/
class MacroParameter : public AutomatableParameter,
                       public AutomatableParameter::ModifierSource
{
public:
    //==============================================================================
    /**
        An Assignment between a MacroParameter and an AutomatableParameter.
    */
    struct Assignment  : public AutomatableParameter::ModifierAssignment
    {
        Assignment (const yup::ValueTree&, const MacroParameter&);

        using Ptr = yup::ReferenceCountedObjectPtr<Assignment>;

        bool isForModifierSource (const AutomatableParameter::ModifierSource&) const override;

        const EditItemID macroParamID;

        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Assignment)
    };

    //==============================================================================
    MacroParameter (AutomatableEditItem&, Edit&, const yup::ValueTree&);
    ~MacroParameter() override;

    using Ptr = yup::ReferenceCountedObjectPtr<MacroParameter>;

    void initialise();

    yup::String getParameterName() const override         { return macroName; }
    std::optional<float> getDefaultValue() const override;

    Edit& edit;
    yup::ValueTree state;
    juce::CachedValue<float> value, defaultValue;
    juce::CachedValue<yup::String> macroName;

private:
    virtual void parameterChanged (float, bool byAutomation) override;
};

/** Searched the Edit for a MacroParameter with this ID. */
MacroParameter::Ptr getMacroParameterForID (Edit&, EditItemID);

//==============================================================================
/** */
class MacroParameterList    : public AutomatableEditItem,
                              public yup::ChangeBroadcaster
{
public:
    MacroParameterList (Edit&, const yup::ValueTree&);
    ~MacroParameterList() override;

    MacroParameter* createMacroParameter();
    void removeMacroParameter (MacroParameter&);
    void hideMacroParametersFromTracks() const;

    yup::ReferenceCountedArray<MacroParameter> getMacroParameters() const;
    void visitMacroParameters (const std::function<void(AutomatableParameter&)>& visit) const;

    yup::String getName() const override     { return {}; }
    Track* getTrack() const;

    //==============================================================================
    void restorePluginStateFromValueTree (const yup::ValueTree&) override {}

    yup::ValueTree state;

private:
    struct List;
    yup::ValueTree parent;
    std::unique_ptr<List> list;
};

/** If this MacroParameterList belongs to an Plugin, this will return it. */
yup::ReferenceCountedObjectPtr<Plugin> getOwnerPlugin (MacroParameterList*);

//==============================================================================
/**
    Base class for elements which can contain macro parameters.
*/
class MacroParameterElement
{
public:
    /** Constructor. */
    MacroParameterElement (Edit&, const yup::ValueTree&);

    /** Destructor. */
    virtual ~MacroParameterElement() = default;

    /** If no parameters have been created, this may return nullptr */
    MacroParameterList* getMacroParameterList();

    /** This ensures that the list has been created */
    MacroParameterList& getMacroParameterListForWriting();

    yup::ReferenceCountedArray<MacroParameter> getMacroParameters() const;

    Edit& ownerEdit;
    yup::ValueTree parentStateForList;

private:
    std::unique_ptr<MacroParameterList> macroParameterList;
};

} // namespace tracktion::inline engine
