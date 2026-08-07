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

static bool isWorthConvertingToOgg (AudioFile& source, int quality)
{
    if (dynamic_cast<yup::OggVorbisAudioFormat*> (source.getFormat()) != nullptr)
    {
        auto estimatedQuality = yup::OggVorbisAudioFormat().estimateOggFileQuality (source.getFile());

        return estimatedQuality == 0
                || quality < estimatedQuality; // if they're asking for a higher quality than the current one, no point in converting
    }

    return true;
}

//==============================================================================
TracktionArchiveFile::IndexEntry::IndexEntry (yup::InputStream& in)
{
    offset = in.readInt();
    length = in.readInt();
    originalName = in.readString();
    storedName = in.readString();

    int numExtras = in.readShort();

    while (--numExtras >= 0 && ! in.isExhausted())
    {
        extraNames.add (in.readString());
        extraValues.add (in.readString());
    }

    DBG (originalName + " / " + storedName + "  length: " << length);
}

TracktionArchiveFile::IndexEntry::~IndexEntry() {}

void TracktionArchiveFile::IndexEntry::write (yup::OutputStream& out)
{
    out.writeInt (int (offset));
    out.writeInt (int (length));
    out.writeString (originalName);
    out.writeString (storedName);

    out.writeShort ((short) extraNames.size());

    for (int i = 0; i < extraNames.size(); ++i)
    {
        out.writeString (extraNames[i]);
        out.writeString (extraValues[i]);
    }
}


//==============================================================================
TracktionArchiveFile::TracktionArchiveFile (Engine& e, const yup::File& f)
    : engine (e), file (f)
{
    readIndex();
}

TracktionArchiveFile::~TracktionArchiveFile()
{
    flush();
}

int TracktionArchiveFile::getMagicNumber()
{
    return (int) yup::ByteOrder::littleEndianInt ("TKNA");
}

yup::File TracktionArchiveFile::getFile() const
{
    return file;
}

void TracktionArchiveFile::readIndex()
{
    valid = false;
    needToWriteIndex = false;
    entries.clear();

    yup::FileInputStream in (file);

    if (in.openedOk())
    {
        const int magic = in.readInt();

        if (magic == getMagicNumber())
        {
            indexOffset = in.readInt();
            jassert (indexOffset >= 8 && indexOffset < file.getSize());

            if (indexOffset >= 8 && indexOffset < file.getSize())
            {
                in.setPosition (indexOffset);
                int numEntries = in.readInt();

                while (--numEntries >= 0 && ! in.isExhausted())
                    entries.add (new IndexEntry (in));

                valid = true;
            }
        }
    }
}

void TracktionArchiveFile::flush()
{
    if (needToWriteIndex && valid)
    {
        yup::FileOutputStream out (file);

        if (out.openedOk())
        {
            out.setPosition (indexOffset);
            out.writeInt (entries.size());

            for (auto e : entries)
                e->write (out);

            out.setPosition (4);
            out.writeInt (int (indexOffset));
        }

        needToWriteIndex = false;
    }
}

bool TracktionArchiveFile::isValidArchive() const
{
    return valid;
}

int TracktionArchiveFile::getNumFiles() const
{
    return entries.size();
}

yup::String TracktionArchiveFile::getOriginalFileName (int index) const
{
    if (auto i = entries[index])
        return i->originalName.fromLastOccurrenceOf ("/", false, false);

    return {};
}

int TracktionArchiveFile::indexOfFile (const yup::String& name) const
{
    for (int i = entries.size(); --i >= 0;)
        if (getOriginalFileName(i).equalsIgnoreCase (name))
            return i;

    return -1;
}

std::unique_ptr<yup::InputStream> TracktionArchiveFile::createStoredInputStream (int index) const
{
    if (entries[index] != nullptr)
        if (auto f = file.createInputStream())
            return std::make_unique<yup::SubregionStream> (f.release(),
                                                            entries[index]->offset,
                                                            entries[index]->length,
                                                            true);

    return {};
}

bool TracktionArchiveFile::extractFile (int index, const yup::File& destDirectory,
                                        yup::File& fileCreated, bool askBeforeOverwriting,
                                        std::function<void (bool)> callback)
{
    if (! destDirectory.createDirectory())
        return false;

    auto destFile = destDirectory.getChildFile (getOriginalFileName (index));
    fileCreated = destFile;

    auto doExtract = [this, index, destFile]() -> bool
    {
        if (destFile.isDirectory()
             || ! destFile.hasWriteAccess()
             || ! destFile.deleteFile()
             || entries[index] == nullptr)
            return false;

        auto source = createStoredInputStream (index);

        if (source == nullptr)
            return false;

        auto storedName = entries[index]->storedName;

        if (storedName != entries[index]->originalName)
        {
            if (storedName.endsWithIgnoreCase (".flac"))
                return AudioFileUtils::readFromFormat<yup::FlacAudioFormat> (engine, *source, destFile);

            if (storedName.endsWithIgnoreCase (".ogg"))
                return AudioFileUtils::readFromFormat<yup::OggVorbisAudioFormat> (engine, *source, destFile);

            if (storedName.endsWithIgnoreCase (".gz"))
                source = std::make_unique<yup::GZIPDecompressorInputStream> (source.release(), true);
            else
                jassertfalse;
        }

        {
            yup::FileOutputStream out (destFile);

            if (! out.openedOk())
                return false;

            out.writeFromInputStream (*source, -1);
        }

        return true;
    };

    if (askBeforeOverwriting && destFile.existsAsFile())
    {
        engine.getUIBehaviour()
            .showYesNoCancelAlertBoxAsync (TRANS("Unpacking archive"),
                                           TRANS("The file XZZX already exists - do you want to overwrite it?")
                                             .replace ("XZZX", destFile.getFullPathName()),
                                           TRANS("Overwrite"),
                                           TRANS("Leave existing"),
                                           TRANS("Cancel"),
                                           [doExtract, callback] (int r)
                                           {
                                               if (r == 1)
                                               {
                                                   if (callback) callback (true);
                                                   return;
                                               }

                                               if (r == 0)
                                               {
                                                   if (callback) callback (false);
                                                   return;
                                               }

                                               // r == 2 means "leave existing" — extract/overwrite
                                               if (callback)
                                                   callback (doExtract());
                                           });

        // Return true as a placeholder; actual result delivered via callback
        return true;
    }

    return doExtract();
}

bool TracktionArchiveFile::extractAll (const yup::File& destDirectory,
                                       yup::Array<yup::File>& filesCreated)
{
    if (! destDirectory.createDirectory())
        return false;

    for (int i = 0; i < entries.size(); ++i)
    {
        yup::File fileCreated;

        if (! extractFile (i, destDirectory, fileCreated, false))
            return false;

        filesCreated.add (fileCreated);
    }

    return true;
}

//==============================================================================
class ExtractionTask  : public ThreadPoolJobWithProgress
{
public:
    ExtractionTask (TracktionArchiveFile& archive_, const yup::File& destDir_,
                    bool warnAboutOverwrite_,
                    yup::Array<yup::File>& filesCreated_,
                    bool& wasAborted_)
        : ThreadPoolJobWithProgress (TRANS("Unpacking archive") + "..."),
          archive (archive_), destDir (destDir_),
          wasAborted (wasAborted_),
          warnAboutOverwrite (warnAboutOverwrite_),
          filesCreated (filesCreated_)
    {
        wasAborted = false;
    }

    JobStatus runJob()
    {
        CRASH_TRACER
        yup::FloatVectorOperations::disableDenormalisedNumberSupport();

        if (auto res = destDir.createDirectory(); res.failed())
        {
            TRACKTION_LOG_ERROR(res.getErrorMessage() + ": " + destDir.getFullPathName());
            return jobHasFinished;
        }

        for (int i = 0; i < archive.getNumFiles(); ++i)
        {
            if (shouldExit())
            {
                wasAborted = true;

                for (auto& f : filesCreated)
                    f.deleteFile();

                break;
            }

            progress = float (i) / float (archive.getNumFiles());

            yup::File fileCreated;

            if (! archive.extractFile (i, destDir, fileCreated, warnAboutOverwrite))
            {
                if (fileCreated != yup::File())
                {
                    TRACKTION_LOG_ERROR("Unable to extract file to: " + fileCreated.getFullPathName());
                }
                else
                {
                    TRACKTION_LOG_ERROR("Unable to extract file to dir: " + destDir.getFullPathName());
                }

                return jobHasFinished;
            }

            if (fileCreated.exists())
                filesCreated.add (fileCreated);
        }

        ok = true;
        return jobHasFinished;
    }

    float getCurrentTaskProgress()
    {
        return progress;
    }

    TracktionArchiveFile& archive;
    yup::File destDir;
    bool ok = false;
    bool& wasAborted;
    float progress = 0;
    bool warnAboutOverwrite = false;
    yup::Array<yup::File>& filesCreated;
};

//==============================================================================
bool TracktionArchiveFile::extractAllAsTask (const yup::File& destDirectory, bool warnAboutOverwrite,
                                             yup::Array<yup::File>& filesCreated, bool& wasAborted)
{
    ExtractionTask task (*this, destDirectory, warnAboutOverwrite, filesCreated, wasAborted);

    engine.getUIBehaviour().runTaskWithProgressBar (task);

    return task.ok;
}

bool TracktionArchiveFile::addFile (const yup::File& f, const yup::File& rootDirectory,
                                    CompressionType compression)
{
    yup::String name;

    if (f.isAChildOf (rootDirectory))
        name = f.getRelativePathFrom (rootDirectory)
                .replaceCharacter ('\\', '/');
    else
        name = f.getFileName();

    return addFile (f, name, compression);
}

bool TracktionArchiveFile::addFile (const yup::File& f, const yup::String& filenameToUse,
                                    CompressionType compression)
{
    // don't risk using ogg or flac on small audio files
    if (compression != CompressionType::none && f.getSize() <= 16 * 1024)
        compression = CompressionType::zip;

    yup::FileInputStream in (f);
    bool ok = false;

    if (in.openedOk())
    {
        yup::FileOutputStream out (file);

        if (out.openedOk())
        {
            if (! valid)
            {
                out.setPosition (0);
                out.writeInt (getMagicNumber());
                out.writeInt (int (indexOffset));
                valid = true;
            }

            auto initialPosition = out.getPosition();

            out.setPosition (indexOffset);
            jassert (indexOffset < 2147483648);

            if (indexOffset >= 2147483648)
            {
                TRACKTION_LOG_ERROR ("Archive too large when archiving file: " + f.getFileName());
                return false;
            }

            auto filenameRoot = filenameToUse.substring (0, filenameToUse.lastIndexOfChar ('.'));

            std::unique_ptr<IndexEntry> entry (new IndexEntry());
            entry->offset = indexOffset;
            entry->length = 0;
            entry->originalName = filenameToUse;
            entry->storedName = filenameToUse;

            switch (compression)
            {
                case CompressionType::none:
                {
                    out.writeFromInputStream (in, -1);
                    break;
                }

                case CompressionType::zip:
                {
                    entry->storedName = filenameRoot + ".gz";

                    yup::GZIPCompressorOutputStream deflater (&out, 9, false);
                    deflater.writeFromInputStream (in, -1);
                    break;
                }

                case CompressionType::lossless:
                {
                    AudioFile af (engine, f);

                    if (af.isOggFile() || af.isMp3File() || af.isFlacFile())
                    {
                        out.writeFromInputStream (in, -1); // no point re-compressing these
                    }
                    else
                    {
                        if (af.getBitsPerSample() > 24)
                        {
                            // FLAC can't do higher than 24 bits so just have to zip it instead..
                            entry->storedName = filenameRoot + ".gz";

                            yup::GZIPCompressorOutputStream deflater (&out, 9, false);
                            deflater.writeFromInputStream (in, -1);
                        }
                        else
                        {
                            entry->storedName = filenameRoot + ".flac";

                            if (! AudioFileUtils::convertToFormat<yup::FlacAudioFormat> (engine, f, out, 0,
                                                                                          yup::StringPairArray()))
                            {
                                needToWriteIndex = true;
                                TRACKTION_LOG_ERROR ("Failed to add file to archive flac: " + f.getFileName());
                                return false;
                            }
                        }
                    }

                    break;
                }

                case CompressionType::lossyGoodQuality:
                case CompressionType::lossyMediumQuality:
                case CompressionType::lossyLowQuality:
                {
                    entry->storedName = filenameRoot + ".ogg";
                    entry->originalName = entry->storedName;  // oggs get extracted as oggs, not named back to how they were

                    auto quality = getOggQuality (compression);
                    AudioFile af (engine, f);

                    if (! isWorthConvertingToOgg (af, quality))
                    {
                        yup::FileInputStream fin (af.getFile());

                        if (! fin.openedOk())
                        {
                            needToWriteIndex = true;
                            TRACKTION_LOG_ERROR ("Failed to add file to archive: " + f.getFileName());
                            return false;
                        }

                        out.writeFromInputStream (fin, -1);
                    }
                    else if (! AudioFileUtils::convertToFormat<yup::OggVorbisAudioFormat> (engine, f, out, quality,
                                                                                            yup::StringPairArray()))
                    {
                        needToWriteIndex = true;
                        TRACKTION_LOG_ERROR ("Failed to add file to archive ogg: " + f.getFileName());
                        return false;
                    }

                    break;
                }

                default:
                {
                    TRACKTION_LOG_ERROR ("Unknown compression type when archiving file: " + f.getFileName());
                    jassertfalse;
                    break;
                }
            }

            out.flush();

            jassert (out.getPosition() > indexOffset);

            entry->length = std::max (yup::int64(), out.getPosition() - indexOffset);

            jassert (indexOffset + entry->length < 2147483648);

            if (indexOffset + entry->length >= 2147483648)
            {
                out.setPosition (initialPosition);
                out.truncate();
                TRACKTION_LOG_ERROR ("Archive too large when archiving file: " + f.getFileName());
                return false;
            }

            indexOffset += entry->length;
            needToWriteIndex = true;
            ok = true;

            entries.add (entry.release());
        }
    }

    return ok;
}

void TracktionArchiveFile::addFileInfo (const yup::String& filename,
                                        const yup::String& itemName,
                                        const yup::String& itemValue)
{
    auto i = indexOfFile (filename);

    if (auto entry = entries[i])
    {
        entry->extraNames.add (itemName);
        entry->extraValues.add (itemValue);
        needToWriteIndex = true;
    }
}

int TracktionArchiveFile::getOggQuality (CompressionType c)
{
    auto numOptions = yup::OggVorbisAudioFormat().getQualityOptions().size();

    if (c == CompressionType::lossyGoodQuality)     return numOptions - 1;
    if (c == CompressionType::lossyMediumQuality)   return numOptions / 2;

    return numOptions / 5;
}

} // namespace legacy
} // namespace tracktion::inline engine
