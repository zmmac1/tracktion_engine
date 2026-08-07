/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

bool isMidiFile (const yup::File&);
bool isTracktionEditFile (const yup::File&);
bool isTracktionArchiveFile (const yup::File&);
bool isTracktionProjectFile (const yup::File&);
bool isTracktionProjectFolder (const yup::File&);
bool isTracktionProjectFileOrFolder (const yup::File&);
bool isTracktionPresetFile (const yup::File&);

const char* const projectFileSuffix = ".tracktion";
const char* const editFileSuffix = ".tracktionedit";
const char* const legacyEditFileSuffix = ".trkedit";
const char* const legacyArchiveFileSuffix = ".trkarch";
const char* const archiveFileSuffix_v2 = ".zip";

const char* const soundFileWildCard = "*.wav;*.aiff;*.aif;*.ogg;*.mp3;*.flac;*.au;*.voc;*.caf;*.w64;*.rx2;*.rcy;*.rex;*.wfaf";
const char* const soundFileAndMidiWildCard = "*.wav;*.aiff;*.aif;*.ogg;*.mp3;*.mid;*.midi;*.flac;*.au;*.voc;*.caf;*.w64;*.rx2;*.rcy;*.rex;*.wfaf";
const char* const midiFileWildCard = "*.midi;*.mid";

const char* const soundFileExtensions = "wav;aiff;aif;ogg;mp3;flac;au;voc;caf;w64;rx2;rcy;rex;wfaf";
const char* const soundFileAndMidiExtensions = "wav;aiff;aif;ogg;mp3;flac;au;voc;caf;w64;rx2;rcy;rex;wfaf;midi;mid";
const char* const midiFileExtensions = "midi;mid";

const char* const keyMapWildCard = "*.tracktionkeys";
const char* const keyMapSuffix = ".tracktionkeys";

const char* const customControllerFileSuffix = ".trkctrlr";
const char* const customProgramSetWildcard = "*.trkmidi;*.midnam";
const char* const customProgramSetFileSuffix = ".trkmidi";
const char* const presetFileSuffix = ".trkpreset";
const char* const rackFileSuffix = ".trkrack";
const char* const scriptFileSuffix = ".tracktionscript";
const char* const grooveTemplateSuffix = ".trkgroove";
const char* const grooveTemplateWildCard = "*.trkgroove";

//==============================================================================
yup::File getNonExistentSiblingWithIncrementedNumberSuffix (const yup::File&, bool addHashSymbol);


//==============================================================================
struct ScopedDirectoryDeleter
{
    ScopedDirectoryDeleter (const yup::File& dir) : f (dir)  {}
    ~ScopedDirectoryDeleter()  { f.deleteRecursively(); }

private:
    yup::File f;
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedDirectoryDeleter)
};

//==============================================================================
struct FileDragList  : public yup::ReferenceCountedObject
{
    enum PreferredLayout
    {
        sameTrack,
        consecutiveTracks,
    };

    PreferredLayout preferredLayout = sameTrack;
    yup::Array<yup::File> files;

    using Ptr = yup::ReferenceCountedObjectPtr<FileDragList>;

    static FileDragList::Ptr getFromDrag (const yup::DragAndDropTarget::SourceDetails&);
    static yup::var create (const yup::Array<yup::File>&, PreferredLayout peferredLayout);
    static yup::var create (const yup::File& file, PreferredLayout peferredLayout);
};

} // namespace tracktion::inline engine
