/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#pragma once

namespace tracktion::inline engine { namespace dawproject {

//==============================================================================
/**
    Internal class for importing DAWproject files into an Edit.
    This handles parsing the XML and constructing Edit objects.
*/
class DAWprojectImporter
{
public:
    DAWprojectImporter (Engine& engine, const ParseOptions& options);

    /** Imports from a .dawproject ZIP file. */
    std::unique_ptr<Edit> importFromFile (const yup::File& file);

    /** Imports from parsed XML elements. */
    std::unique_ptr<Edit> importFromXml (const yup::XmlElement& projectXml,
                                         const yup::File& audioFileDirectory);

private:
    //==============================================================================
    // Structure parsing
    void parseStructure (const yup::XmlElement& structureElement, Edit& edit);
    void parseTrack (const yup::XmlElement& trackElement, Edit& edit, Track* parentTrack);
    void parseChannel (const yup::XmlElement& channelElement, Edit& edit, Track* track);

    // Transport/tempo parsing
    void parseTransport (const yup::XmlElement& transportElement, Edit& edit);
    void parseTempoAutomation (const yup::XmlElement& tempoAutomation, Edit& edit);
    void parseTimeSignatureAutomation (const yup::XmlElement& timeSigAutomation, Edit& edit);

    // Arrangement parsing
    void parseArrangement (const yup::XmlElement& arrangementElement, Edit& edit);
    void parseLanes (const yup::XmlElement& lanesElement, Edit& edit, Track* track, bool positionIsBeats = false);
    void parseClips (const yup::XmlElement& clipsElement, Edit& edit, ClipOwner& target, bool positionIsBeats = false);
    void parseMarkers (const yup::XmlElement& markersElement, Edit& edit);

    // Scenes / clip-launcher parsing
    void parseScenes (const yup::XmlElement& scenesElement, Edit& edit);
    void parseSceneContent (const yup::XmlElement& contentElement, Edit& edit, int sceneIndex);

    // Clip parsing
    Clip::Ptr parseClip (const yup::XmlElement& clipElement, Edit& edit, ClipOwner& target, bool positionIsBeats = false);
    void parseAudioClip (const yup::XmlElement& clipElement, WaveAudioClip& clip);
    void parseMidiClip (const yup::XmlElement& clipElement, MidiClip& clip);
    void parseNotes (const yup::XmlElement& notesElement, MidiClip& clip);
    void parseControllerPoints (const yup::XmlElement& lanesElement, MidiClip& clip);

    // Plugin parsing
    void parseDevices (const yup::XmlElement& devicesElement, Edit& edit, Track& track);
    Plugin::Ptr parsePlugin (const yup::XmlElement& pluginElement, Edit& edit);

    // Automation parsing
    void parseAutomationPoints (const yup::XmlElement& pointsElement, AutomatableParameter& param);

    // Audio file handling
    yup::File resolveAudioFile (const yup::String& path, bool external);
    void extractAudioFiles (yup::ZipFile& zipFile, const yup::File& destDir);

    //==============================================================================
    // Utility functions
    TimePosition parseTime (const yup::XmlElement& element, const char* attrName, bool isBeats) const;
    TimeDuration parseDuration (const yup::XmlElement& element, const char* attrName, bool isBeats) const;
    bool isTimeUnitBeats (const yup::XmlElement& element) const;

    //==============================================================================
    Engine& engine;
    ParseOptions options;
    IDRefResolver idResolver;
    yup::File audioFileDirectory;
    yup::File sourceFileDirectory;

    // Maps DAWproject track IDs to created tracks
    std::unordered_map<yup::String, Track*> idToTrack;

    // Maps DAWproject channel IDs to created tracks (for routing)
    std::unordered_map<yup::String, Track*> idToChannel;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DAWprojectImporter)
};

}} // namespace tracktion::inline engine::dawproject
