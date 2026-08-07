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
*/
class GrooveTemplate
{
public:
    GrooveTemplate();
    GrooveTemplate (const yup::XmlElement*);
    GrooveTemplate (const GrooveTemplate&);
    ~GrooveTemplate();

    const GrooveTemplate& operator= (const GrooveTemplate&);
    bool operator== (const GrooveTemplate&) const;

    //==============================================================================
    bool isEmpty() const;
    bool isParameterized() const;
    void setParameterized (bool p)                          { parameterized = p; }

    //==============================================================================
    /** Apply this groove to a time, in beats */
    BeatPosition beatsTimeToGroovyTime (BeatPosition beatsTime, float strength) const;

    /** Apply this groove to a time, in seconds */
    TimePosition editTimeToGroovyTime (TimePosition editTime, float strength, Edit& edit) const;

    //==============================================================================
    const yup::String& getName() const                     { return name; }
    void setName (const yup::String&);

    // all indexes are in quarter-notes
    int getNumberOfNotes() const                            { return numNotes; }
    void setNumberOfNotes (int notes);

    int getNotesPerBeat() const                             { return notesPerBeat; }
    void setNotesPerBeat (int notes);

    // value is 0 to 1.0 of the length of a note
    float getLatenessProportion (int noteNumber, float strength) const;
    void setLatenessProportion (int noteNumber, float p, float strength);
    void clearLatenesses();

    //==============================================================================
    static const char* grooveXmlTag;
    yup::XmlElement* createXml() const;

private:
    //==============================================================================
    yup::Array<float> latenesses;
    int numNotes, notesPerBeat;
    yup::String name;
    bool parameterized = false;

    //==============================================================================
    YUP_LEAK_DETECTOR (GrooveTemplate)
};

//==============================================================================
/**
    Looks after the list of groove templates.
*/
class GrooveTemplateManager
{
public:
    //==============================================================================
    GrooveTemplateManager (Engine&);

    void useParameterizedGrooves (bool b);

    //==============================================================================
    int getNumTemplates() const;
    yup::String getTemplateName (int index) const;

    yup::StringArray getTemplateNames() const;

    const GrooveTemplate* getTemplate (int index);
    const GrooveTemplate* getTemplateByName (const yup::String& name);

    void updateTemplate (int index, const GrooveTemplate&);
    void deleteTemplate (int index);

    /** called when usersettings change, because that's where the grooves are kept. */
    void reload();

    Engine& engine;

private:
    //==============================================================================
    yup::OwnedArray<GrooveTemplate> knownGrooves;
    yup::Array<GrooveTemplate*> activeGrooves;

    //==============================================================================
    void save();
    void reload (const yup::XmlElement*);

    bool useParameterized = false;
    //==============================================================================
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrooveTemplateManager)
};

} // namespace tracktion::inline engine
