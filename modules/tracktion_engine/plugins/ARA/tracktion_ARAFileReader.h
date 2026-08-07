/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#ifndef T_ARA_DBG
 // #define T_ARA_DBG(x) DBG(x)
 #define T_ARA_DBG(...)
#endif

namespace tracktion::inline engine {

struct ARAClipPlayer;

//==============================================================================
/**
    Manages an ARA plugin instance for an audio clip.

    This class handles the ARA (Audio Random Access) integration, allowing
    plugins like Melodyne to provide advanced audio analysis and manipulation.
*/
class ARAFileReader   : public yup::ReferenceCountedObject,
                        public yup::ChangeBroadcaster
{
public:
    ARAFileReader (Edit&, AudioClipBase&);
    ARAFileReader (Edit&, AudioClipBase&, ARAFileReader& oneToClone);
    ~ARAFileReader();

    using Ptr = yup::ReferenceCountedObjectPtr<ARAFileReader>;

    static void cleanUpOnShutdown();

    //==============================================================================
    /** Parsed info from an ARA audio file chunk embedded in iXML. */
    struct ARAChunkInfo
    {
        yup::String documentArchiveID;
        bool openAutomatically = false;
        yup::String persistentID;
        yup::MemoryBlock archiveData;
        yup::String suggestedPlugInName;
        yup::String manufacturerName;
    };

    /** Reads the raw iXML chunk from a WAV or AIFF file. */
    static yup::String readRawIXMLFromSourceFile (const yup::File&);

    /** Parses ARA audio file chunks from a raw iXML string. */
    static yup::Array<ARAChunkInfo> parseARAAudioFileChunksFromIXML (const yup::String& ixmlString);

    /** Finds an installed ARA plugin whose factory matches the given archive ID.
        If suggestedPlugInName is given (e.g. from an ARA audio file chunk), only
        plugins with that name are considered - this avoids loading the module of
        every installed ARA plugin just to compare archive IDs. */
    static yup::PluginDescription findPluginForARAArchiveID (Engine&, const yup::String& archiveID,
                                                              const yup::String& suggestedPlugInName = {});

    //==============================================================================
    bool isValid() const noexcept                       { return player != nullptr; }

    ExternalPlugin* getPlugin();
    void showPluginWindow();
    void hidePluginWindow();

    /** Sends the clip's playback regions and region sequence as the current view
        selection to every instance bound to the same document whose plugin UI is
        currently showing (other clips' instances and e.g. the browser panel
        instance). Call when the arrangement selection changes so open plugin UIs
        can follow it. */
    void notifyViewSelection();

    /** Returns the number of ARA playback regions currently created for the clip
        (one per loop repeat for looping clips). */
    int getNumPlaybackRegions() const;

    bool isAnalysingContent();
    yup::MidiMessageSequence getAnalysedMIDISequence();

    void sourceClipChanged();

    /** Notifies that the ARA content has changed (e.g. notes edited in Melodyne).
        This re-reads the content and broadcasts a change message. */
    void contentHasChanged();

    /** Store a partial ARA archive of this clip's plugin edits for copy/paste.
        Returns an empty MemoryBlock if ARA is not active. */
    yup::MemoryBlock storeARAArchiveForCopy();

    /** Restore a partial ARA archive into this clip after paste.
        @param data            The archive data (from storeARAArchiveForCopy)
        @param archivedSourceID  The audio source persistent ID from when the archive was created
        @param archivedModID     The audio modification persistent ID from when the archive was created
    */
    void restoreARAArchiveForPaste (const yup::MemoryBlock& data,
                                    const yup::String& archivedSourceID,
                                    const yup::String& archivedModID,
                                    const yup::String& documentArchiveID = {});

    /** Returns the persistent ID of the current audio source, or empty if ARA is not active. */
    yup::String getAudioSourcePersistentID() const;

    /** Returns the persistent ID of the current audio modification, or empty if ARA is not active. */
    yup::String getAudioModificationPersistentID() const;

    /** Returns the document archive ID from the ARA factory, or empty if ARA is not active. */
    yup::String getDocumentArchiveID() const;

    /** Returns extra time before the playback region start reported by the ARA plugin. */
    TimeDuration getHead() const;

    /** Returns extra time after the playback region end reported by the ARA plugin. */
    TimeDuration getTail() const;

    /** Serialises audio-thread rendering of the plugin against message-thread renderer
        deactivation (ARA playback-region changes require the plugin to not be in
        render-state). The audio thread try-locks this and outputs silence if held. */
    yup::CriticalSection& getProcessLock() noexcept    { return processLock; }

private:
    std::unique_ptr<ARAClipPlayer> player;
    yup::MidiBuffer midiBuffer;
    yup::CriticalSection processLock;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAFileReader)
};

/** @deprecated Use ARAFileReader instead */
using MelodyneFileReader = ARAFileReader;

//==============================================================================
/** Result of detecting ARA iXML chunks in an audio file. */
struct ARAIXMLResult
{
    yup::PluginDescription pluginDescription;
    yup::MemoryBlock archiveData;
    yup::String persistentID;
    yup::String documentArchiveID;
    bool isValid() const { return pluginDescription.name.isNotEmpty(); }
};

/** Scans a source audio file for ARA iXML chunks and returns the first
    matching installed plugin description along with archive data. */
ARAIXMLResult detectARAFromIXMLChunks (Engine&, const yup::File& sourceFile);


//==============================================================================
/** RAII wrapper for an ARA document-controller binding created by
    ARADocumentHolder::bindPluginToDocument, owned by the bound ExternalPlugin
    (see ExternalPlugin::setARADocumentBinding). It's destroyed - unregistering
    from the ARA document - just before the plugin's instance is deleted, which
    is when the bind itself actually ends. */
class ARAPluginBinding
{
public:
    ~ARAPluginBinding();

private:
    friend struct ARADocumentHolder;
    struct Impl;
    explicit ARAPluginBinding (std::unique_ptr<Impl>);

    std::unique_ptr<Impl> impl;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginBinding)
};

//==============================================================================
struct ARADocumentHolder
{
    ARADocumentHolder (Edit&, const yup::ValueTree&);
    ~ARADocumentHolder();

    void flushStateToValueTree();

    /** Binds an already-loaded, ARA-capable ExternalPlugin to this Edit's ARA document
        controller for the given description (lazily creating the document), putting the
        plugin into ARA mode. No audio source, playback region or musical context is created.

        The plugin takes ownership of the binding (ExternalPlugin::setARADocumentBinding),
        which lasts for the lifetime of its loaded instance - if the plugin is already
        bound (e.g. it came back from the PluginCache), the existing binding is kept.

        Returns false if the plugin isn't ARA-capable, its instance isn't loaded yet, or
        the binding fails. */
    bool bindPluginToDocument (ExternalPlugin&, const yup::PluginDescription&);

    /** Notifies every existing ARA document that the edit's tempo/key/chord content
        has changed so the plugins re-read their musical contexts. Called from
        Edit::sendTempoOrPitchSequenceChangedUpdates so it also covers documents with
        no clips (e.g. a browser/panel-only instance - QA 16550). Doesn't trigger
        document creation. */
    void musicalContextContentChanged();

    struct Pimpl;
    Pimpl* getPimpl();

private:
    Edit& edit;
    yup::ValueTree lastState;
    std::unique_ptr<Pimpl> pimpl;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentHolder)
};

} // namespace tracktion::inline engine
