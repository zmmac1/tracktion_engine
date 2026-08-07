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
class MidiProgramManager
{
public:
    MidiProgramManager (Engine&);

    yup::StringArray getMidiProgramSetNames();
    yup::String getProgramName (int set, int bank, int program);
    yup::String getBankName (int set, int bank);
    int getBankID (int set, int bank);
    void setBankID (int set, int bank, int id);
    void setProgramName (int set, int bank, int program, const yup::String& newName);
    void setBankName (int set, int bank, const yup::String& newName);
    void clearProgramName (int set, int bank, int program);
    yup::String getDefaultName (int bank, int program, bool zeroBased);
    yup::String getDefaultCustomName();
    int getSetIndex (const yup::String& setName);
    bool isZeroBased (int set);
    void setZeroBased (int set, bool zeroBased);

    bool doesSetExist (const yup::String& name) const noexcept;
    bool canEditProgramSet (int set) const noexcept;
    bool canDeleteProgramSet (int set) const noexcept;
    void addNewSet (const yup::String& name);
    void addNewSet (const yup::String& name, const yup::XmlElement&);
    void deleteSet (int set);
    bool importSet (int set, const yup::File&);
    bool exportSet (int set, const yup::File&);

    static yup::StringArray getListOfPresets (Engine&);
    yup::String getPresetXml (yup::String presetName);

    void saveAll();

    Engine& engine;

protected:
    struct MidiBank
    {
        yup::XmlElement* createXml();
        void updateFromXml (const yup::XmlElement&);

        yup::String name;
        int id = 0;
        std::map<int, yup::String> programNames;
    };

    struct MidiProgramSet
    {
        MidiProgramSet();
        yup::XmlElement* createXml();
        void updateFromXml (const yup::XmlElement&);

        yup::String name;
        bool zeroBased = true;
        MidiBank midiBanks[16];
    };

    yup::OwnedArray<MidiProgramSet> programSets;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiProgramManager)
};

} // namespace tracktion::inline engine
