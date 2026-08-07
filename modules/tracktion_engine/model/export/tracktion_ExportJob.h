/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {
namespace legacy {

//==============================================================================
/**
*/
class ExportJob  : public ThreadPoolJobWithProgress
{
public:
    enum class CompressionLevel
    {
        lossless = 0,
        highQuality,
        mediumQuality,
        lowestQuality,
        uncompressed
    };

    ExportJob (Edit*,
               const yup::File& destDir,
               const Project::Ptr& newProject,
               const Project::Ptr& srcProject,
               TracktionArchiveFile*,
               double handleSize_,
               bool keepEntireFiles,
               TracktionArchiveFile::CompressionType,
               yup::Array<yup::File>& filesForDeletion,
               yup::StringArray& failedFiles,
               bool includeLibraryFiles,
               bool includeClips);

    ~ExportJob() override;

    //==============================================================================
    JobStatus runJob() override;
    float getCurrentTaskProgress() override;

private:
    Edit* edit = nullptr;
    float progress = 0;
    yup::File destDir;
    Project::Ptr newProject, srcProject;
    double handleSize = 0;
    bool keepEntireFiles = false;
    TracktionArchiveFile::CompressionType compressionType;
    TracktionArchiveFile* archive = nullptr;
    yup::Array<yup::File>& filesForDeletion;
    yup::OwnedArray<Edit> edits;
    yup::StringArray& failedFiles;
    bool includeLibraryFiles = false;
    bool includeClips = false;

    void copyEditFilesToTempDir();
    void copyProjectFilesToTempDir();
    void createArchiveFromTempFiles();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExportJob)
};

} // namespace legacy
} // namespace tracktion::inline engine
