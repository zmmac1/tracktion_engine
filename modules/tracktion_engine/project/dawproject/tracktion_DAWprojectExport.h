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
    Internal class for exporting an Edit to DAWproject format.
    This handles generating XML and writing the ZIP archive.
*/
class DAWprojectExporter
{
public:
    DAWprojectExporter (Edit& edit, const WriteOptions& options);

    /** Creates the project.xml content. */
    tl::expected<std::unique_ptr<yup::XmlElement>, yup::String> createProjectXml();

    /** Creates the metadata.xml content. */
    std::unique_ptr<yup::XmlElement> createMetadataXml();

    /** Writes the complete .dawproject file. */
    yup::Result writeToFile (const yup::File& file);

private:
    //==============================================================================
    // Structure export
    yup::XmlElement* createStructure (yup::XmlElement& parent);
    yup::XmlElement* createTrackElement (yup::XmlElement& parent, Track& track);
    yup::XmlElement* createChannelElement (yup::XmlElement& parent, Track& track);

    // Transport/tempo export
    yup::XmlElement* createTransport (yup::XmlElement& parent);
    yup::XmlElement* createTempoAutomation (yup::XmlElement& parent);
    yup::XmlElement* createTimeSignatureAutomation (yup::XmlElement& parent);

    // Arrangement export
    yup::XmlElement* createArrangement (yup::XmlElement& parent);
    yup::XmlElement* createLanes (yup::XmlElement& parent, Track& track);
    yup::XmlElement* createClipsElement (yup::XmlElement& parent, ClipTrack& track);
    yup::XmlElement* createMarkersElement (yup::XmlElement& parent);

    // Scenes (clip launcher) export
    yup::XmlElement* createScenes (yup::XmlElement& parent);

    // Clip export
    yup::XmlElement* createClipElement (yup::XmlElement& parent, Clip& clip);
    void exportAudioClip (yup::XmlElement& clipElement, WaveAudioClip& clip);
    void exportMidiClip (yup::XmlElement& clipElement, MidiClip& clip);
    yup::XmlElement* createNotesElement (yup::XmlElement& parent, MidiClip& clip);
    yup::XmlElement* createNoteElement (yup::XmlElement& parent, const MidiNote& note);
    void createControllerLanes (yup::XmlElement& parent, MidiClip& clip);

    // Plugin export
    yup::XmlElement* createDevices (yup::XmlElement& parent, Track& track);
    yup::XmlElement* createPluginElement (yup::XmlElement& parent, Plugin& plugin);

    // Automation export
    yup::XmlElement* createAutomationPoints (yup::XmlElement& parent, AutomatableParameter& param);

    // Audio file handling
    yup::String addAudioFile (const yup::File& file);
    void writeAudioFiles (yup::ZipFile::Builder& zipBuilder);
    void writePluginStates (yup::ZipFile::Builder& zipBuilder);

    //==============================================================================
    // Utility functions
    yup::String getTrackContentType (Track& track) const;
    yup::String getMixerRole (Track& track) const;
    yup::String getPluginType (Plugin& plugin) const;
    yup::String getDeviceRole (Plugin& plugin) const;

    //==============================================================================
    Edit& edit;
    WriteOptions options;
    IDGenerator idGenerator;

    // Maps tracks to their generated IDs
    std::unordered_map<Track*, yup::String> trackToId;
    std::unordered_map<Track*, yup::String> trackToChannelId;

    // Audio files to embed
    struct AudioFileEntry
    {
        yup::File sourceFile;
        yup::String archivePath;
    };
    std::vector<AudioFileEntry> audioFilesToEmbed;

    // Plugin states to embed
    struct PluginStateEntry
    {
        yup::MemoryBlock state;
        yup::String archivePath;
    };
    std::vector<PluginStateEntry> pluginStatesToEmbed;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DAWprojectExporter)
};

}} // namespace tracktion::inline engine::dawproject
