/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

TemporaryFileManager::TemporaryFileManager (Engine& e)  : engine (e)
{
    updateDir();
}

TemporaryFileManager::~TemporaryFileManager()
{
}

static yup::File getDefaultTempFolder (Engine& engine)
{
    return engine.getPropertyStorage().getAppCacheFolder().getChildFile ("Temporary");
}

static bool isDangerousPath (const yup::File& f)
{
    if (! f.isDirectory() && ! f.getParentDirectory().isDirectory())
        return false;

    // Root directory
    if (f.getParentDirectory() == f)
        return true;

    using SL = yup::File::SpecialLocationType;

    for (auto loc : { SL::userHomeDirectory,
                      SL::userDesktopDirectory,
                      SL::userDocumentsDirectory,
                      SL::userMusicDirectory,
                      SL::userMoviesDirectory,
                      SL::userPicturesDirectory,
                      SL::userApplicationDataDirectory,
                      SL::globalApplicationsDirectory
                     #if JUCE_WINDOWS
                      , SL::windowsSystemDirectory
                     #endif
                    })
    {
        auto special = yup::File::getSpecialLocation (loc);

        if (special == f)
            return true;
    }

    // Also check Downloads (no JUCE constant for this)
    auto downloads = yup::File::getSpecialLocation (SL::userHomeDirectory).getChildFile ("Downloads");

    if (downloads == f)
        return true;

    return false;
}

const yup::File& TemporaryFileManager::getTempDirectory() const
{
    return tempDir;
}

bool TemporaryFileManager::setTempDirectory (const yup::File& newFile)
{
    if (isDangerousPath (newFile))
    {
        TRACKTION_LOG_ERROR ("Refusing to set temp directory to a system folder: " + newFile.getFullPathName());
        return false;
    }

    auto defaultDir = getDefaultTempFolder (engine);

    if (newFile == defaultDir)
    {
        ressetToDefaultLocation();
    }
    else
    {
        auto path = newFile.getFullPathName();
        auto parent = defaultDir.getParentDirectory();

        if (newFile.isAChildOf (parent))
            path = newFile.getRelativePathFrom (parent);

        engine.getPropertyStorage().setProperty (SettingID::tempDirectory, path);
        updateDir();

        jassert (newFile == tempDir);
    }

    return wasTempFolderSuccessfullyCreated();
}

void TemporaryFileManager::ressetToDefaultLocation()
{
    tempDir = getDefaultTempFolder (engine);
    engine.getPropertyStorage().removeProperty (SettingID::tempDirectory);
}

void TemporaryFileManager::updateDir()
{
    auto defaultDir = getDefaultTempFolder (engine);
    auto userFolder = engine.getPropertyStorage().getProperty (SettingID::tempDirectory).toString().trim();

    if (userFolder.isEmpty())
        ressetToDefaultLocation();
    else
        tempDir = defaultDir.getSiblingFile (userFolder);

    if (isDangerousPath (tempDir))
    {
        TRACKTION_LOG_ERROR ("Temp directory resolved to a system folder: " + tempDir.getFullPathName() + " — resetting to default");
        ressetToDefaultLocation();
        return;
    }

    if (! wasTempFolderSuccessfullyCreated())
    {
        tempDir = defaultDir;
        engine.getPropertyStorage().removeProperty (SettingID::tempDirectory);
    }
}

//==============================================================================
bool TemporaryFileManager::wasTempFolderSuccessfullyCreated() const
{
    return (tempDir.isDirectory() || tempDir.createDirectory())
             && tempDir.hasWriteAccess();
}

bool TemporaryFileManager::isDiskSpaceDangerouslyLow() const
{
    auto freeMb = tempDir.getBytesFreeOnVolume() / (1024 * 1024);
    return freeMb < 80;
}

int64_t TemporaryFileManager::getMaxSpaceAllowedForTempFiles() const
{
    int64_t minAbsoluteSize = 1024 * 1024 * 750;
    int64_t minProportionOfDisk = tempDir.getBytesFreeOnVolume() / 4;

    return std::min (minProportionOfDisk, minAbsoluteSize);
}

int TemporaryFileManager::getMaxNumTempFiles() const
{
    return 1000;
}

static bool isRecognisedTempFile (const yup::File&);
static bool shouldDeleteTempFile (const yup::File&, bool);

static void deleteEditPreviewsNotInUse (Engine& engine, yup::Array<yup::File>& files)
{
    auto& pm = engine.getProjectManager();
    yup::StringArray ids;

    for (auto& p : pm.getAllProjects (pm.folders))
        for (auto item : p->getAllProjectItems())
            ids.add (item->getProjectItemRef().toIDForFilename());

    for (int i = files.size(); --i >= 0;)
    {
        auto fileName = files.getUnchecked (i).getFileNameWithoutExtension();

        if (! fileName.startsWith ("preview_"))
            continue;

        auto itemID = fileName.fromFirstOccurrenceOf ("preview_", false, false);

        if (itemID.isNotEmpty() && ! ids.contains (itemID))
            files.removeAndReturn (i).deleteFile();
    }
}

void TemporaryFileManager::cleanUp()
{
    CRASH_TRACER
    TRACKTION_LOG ("Cleaning up temp files..");

    if (isDangerousPath (tempDir))
    {
        TRACKTION_LOG_ERROR ("Skipping temp cleanup — temp directory is a system folder: " + tempDir.getFullPathName());
        return;
    }

    // Only search immediate children and one level of known subdirs (edit_*, thumbnails)
    // to avoid catastrophic enumeration if the temp dir is misconfigured
    auto tempFiles = tempDir.findChildFiles (yup::File::findFiles, false);

    for (auto entry : yup::RangedDirectoryIterator (tempDir, false, "*", yup::File::findDirectories))
        tempFiles.addArray (entry.getFile().findChildFiles (yup::File::findFiles, false));

    deleteEditPreviewsNotInUse (engine, tempFiles);

    int64_t totalBytes = 0;

    for (auto& f : tempFiles)
        totalBytes += f.getSize();

    std::sort (tempFiles.begin(), tempFiles.end(),
               [] (const yup::File& first, const yup::File& second) -> bool
               {
                   return first.getLastAccessTime().toMilliseconds() < second.getLastAccessTime().toMilliseconds();
               });

    auto numFiles = tempFiles.size();
    auto maxNumFiles = getMaxNumTempFiles();
    auto maxSizeToKeep = getMaxSpaceAllowedForTempFiles();

    for (auto& f : tempFiles)
    {
        if (shouldDeleteTempFile (f, totalBytes > maxSizeToKeep || numFiles > maxNumFiles))
        {
            totalBytes -= f.getSize();
            f.deleteFile();
            --numFiles;
        }
    }

    for (auto entry : yup::RangedDirectoryIterator (getTempDirectory(), false, "edit_*", yup::File::findDirectories))
        if (entry.getFile().getNumberOfChildFiles (yup::File::findFilesAndDirectories) == 0)
            entry.getFile().deleteRecursively();
}

yup::File TemporaryFileManager::getTempFile (const yup::String& filename) const
{
    return tempDir.getChildFile (filename);
}

yup::File TemporaryFileManager::getUniqueTempFile (const yup::String& prefix, const yup::String& ext) const
{
    return tempDir.getChildFile (prefix + yup::String::toHexString (yup::Random::getSystemRandom().nextInt64()))
                  .withFileExtension (ext)
                  .getNonexistentSibling();
}

yup::File TemporaryFileManager::getThumbnailsFolder() const
{
    return getTempDirectory().getChildFile ("thumbnails");
}

//==============================================================================
static yup::String getClipProxyPrefix()                { return "clip_"; }
static yup::String getFileProxyPrefix()                { return "proxy_"; }
static yup::String getDeviceFreezePrefix (Edit& edit)  { return "freeze_" + edit.getProjectItemRef().toIDForFilename() + "_"; }
static yup::String getTrackFreezePrefix()              { return "trackFreeze_"; }
static yup::String getCompPrefix()                     { return "comp_"; }

static bool isRecognisedTempFile (const yup::File& f)
{
    auto fileName = f.getFileName();

    if (fileName.startsWith ("temp_")
         || fileName.startsWith ("preview_")
         || fileName.startsWith (getClipProxyPrefix())
         || fileName.startsWith (getFileProxyPrefix())
         || fileName.startsWith (getTrackFreezePrefix())
         || fileName.startsWith ("freeze_")
         || fileName.startsWith (getCompPrefix())
         || fileName.startsWith (RenderManager::getFileRenderPrefix()))
        return true;

    // Files inside edit_* or thumbnails subdirectories are temp files
    auto parentName = f.getParentDirectory().getFileName();

    if (parentName.startsWith ("edit_") || parentName == "thumbnails")
        return true;

    return false;
}

static bool shouldDeleteTempFile (const yup::File& f, bool spaceIsShort)
{
    if (! isRecognisedTempFile (f))
        return false;

    auto fileName = f.getFileName();

    if (fileName.startsWith ("preview_"))
        return false;

    if (fileName.startsWith ("temp_"))
        return true;

    auto daysOld = (yup::Time::getCurrentTime() - f.getLastAccessTime()).inDays();

    return daysOld > 60.0 || (spaceIsShort && daysOld > 1.0);
}

static AudioFile getCachedEditFile (Edit& edit, const yup::String& prefix, HashCode hash)
{
    return AudioFile (edit.engine, edit.getTempDirectory (true).getChildFile (prefix + yup::String::toHexString (hash) + ".wav"));
}

static AudioFile getCachedClipFileWithPrefix (const AudioClipBase& clip, const yup::String& prefix, HashCode hash)
{
    return getCachedEditFile (clip.edit, prefix + "0_" + clip.itemID.toString() + "_", hash);
}

AudioFile TemporaryFileManager::getFileForCachedClipRender (const AudioClipBase& clip, HashCode hash)
{
    return getCachedClipFileWithPrefix (clip, getClipProxyPrefix(), hash);
}

AudioFile TemporaryFileManager::getFileForCachedCompRender (const AudioClipBase& clip, HashCode takeHash)
{
    return getCachedClipFileWithPrefix (clip, getCompPrefix(), takeHash);
}

AudioFile TemporaryFileManager::getFileForCachedFileRender (Edit& edit, HashCode hash)
{
    return getCachedEditFile (edit, getFileProxyPrefix(), hash);
}

yup::File TemporaryFileManager::getFreezeFileForDevice (Edit& edit, OutputDevice& device)
{
    return edit.getTempDirectory (true)
             .getChildFile (getDeviceFreezePrefix (edit) + yup::String (device.getDeviceID()) + ".freeze");
}

yup::String TemporaryFileManager::getDeviceIDFromFreezeFile (Edit& edit, const yup::File& deviceFreezeFile)
{
    const auto fileName = deviceFreezeFile.getFileName();
    jassert (fileName.startsWith (getDeviceFreezePrefix (edit)));
    jassert (fileName.endsWith (".freeze"));

    return fileName.fromLastOccurrenceOf (getDeviceFreezePrefix (edit), false, false)
                   .upToFirstOccurrenceOf (".", false, false);
}

yup::File TemporaryFileManager::getFreezeFileForTrack (const AudioTrack& track)
{
    return track.edit.getTempDirectory (true)
             .getChildFile (getTrackFreezePrefix() + "0_" + track.itemID.toString() + ".freeze");
}

yup::Array<yup::File> TemporaryFileManager::getFrozenTrackFiles (Edit& edit)
{
    return edit.getTempDirectory (false)
             .findChildFiles (yup::File::findFiles, false, getDeviceFreezePrefix (edit) + "*");
}

static ProjectItemID getProjectItemIDFromFilename (const yup::String& name)
{
    auto tokens = yup::StringArray::fromTokens (name.fromFirstOccurrenceOf ("_", false, false), "_", {});

    if (tokens.size() <= 1)
        return {};

    return ProjectItemID (tokens[0] + "_" + tokens[1]);
}

void TemporaryFileManager::purgeOrphanEditTempFolders (ProjectManager& pm)
{
    CRASH_TRACER
    yup::Array<yup::File> filesToDelete;
    yup::StringArray reasonsForDeletion;

    for (auto entry : yup::RangedDirectoryIterator (getTempDirectory(), false, "edit_*", yup::File::findDirectories))
    {
        auto itemID = getProjectItemIDFromFilename (entry.getFile().getFileName());

        if (itemID.isValid())
        {
            if (pm.getProjectItem (itemID) == nullptr)
            {
                filesToDelete.add (entry.getFile());

                auto pp = pm.getProject (itemID.getProjectID());

                if (! itemID.getProjectID().isValid())
                    reasonsForDeletion.add ("Invalid project ID");
                else if (pp == nullptr)
                    reasonsForDeletion.add ("Can't find project");
                else if (pp->getProjectItemFor (itemID) == nullptr)
                    reasonsForDeletion.add ("Can't find source media");
                else
                    reasonsForDeletion.add ("Unknown");
            }
        }
    }

    for (int i = filesToDelete.size(); --i >= 0;)
    {
        TRACKTION_LOG ("Purging edit folder: " + filesToDelete.getReference (i).getFileName() + " - " + reasonsForDeletion[i]);
        filesToDelete.getReference (i).deleteRecursively();
    }
}

static EditItemID getEditItemIDFromFilename (const yup::String& name)
{
   auto tokens = yup::StringArray::fromTokens (name.fromFirstOccurrenceOf ("_", false, false), "_", {});

    if (tokens.isEmpty())
        return {};

    // TODO: think about backwards-compatibility for these strings - was a two-part
    // ID separated by an underscore
    return EditItemID::fromVar (tokens[0] + tokens[1]);
}

void TemporaryFileManager::purgeOrphanFreezeAndProxyFiles (Edit& edit)
{
    CRASH_TRACER
    yup::Array<yup::File> filesToDelete;

    for (auto entry : yup::RangedDirectoryIterator (edit.getTempDirectory (false), false, "*"))
    {
        auto name = entry.getFile().getFileName();
        auto itemID = getEditItemIDFromFilename (name);

        if (itemID.isValid())
        {
            if (name.startsWith (getClipProxyPrefix())
                 || name.startsWith (getCompPrefix()))
            {
                auto clip = findClipForID (edit, itemID);

                if (auto acb = dynamic_cast<AudioClipBase*> (clip))
                {
                    if (! acb->isUsingFile (AudioFile (edit.engine, entry.getFile())))
                        filesToDelete.add (entry.getFile());
                }
                else if (clip == nullptr)
                {
                    filesToDelete.add (entry.getFile());
                }
            }
            else if (name.startsWith (getFileProxyPrefix()))
            {
                // XXX
            }
            else if (name.startsWith (getTrackFreezePrefix()))
            {
                if (auto at = findAudioTrackForID (edit, itemID))
                    if (! at->isFrozen (Track::individualFreeze))
                        filesToDelete.add (entry.getFile());
            }
        }
        else if (name.startsWith (RenderManager::getFileRenderPrefix()))
        {
            if (! edit.areAnyClipsUsingFile (AudioFile (edit.engine, entry.getFile())))
                filesToDelete.add (entry.getFile());
        }
    }

    for (auto& f : filesToDelete)
    {
        DBG ("Purging temp file: " << f.getFileName());
        AudioFile (edit.engine, f).deleteFile();
    }
}

} // namespace tracktion::inline engine
