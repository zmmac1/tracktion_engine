/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

#if TRACKTION_ENABLE_FFMPEG

static yup::String readOutputFromSystem (yup::String cmd)
{
    yup::TemporaryFile tmpFile;
    yup::TemporaryFile batFile (".bat");
    
   #if JUCE_WINDOWS
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory (&si, sizeof (si));
    si.cb = sizeof (si);
    ZeroMemory (&pi, sizeof (pi));

    cmd = cmd + " > " + tmpFile.getFile().getFullPathName().quoted();

    if (cmd.length() > 32768)
    {
        jassertfalse;
        return {};
    }

    batFile.getFile ().replaceWithText (cmd);

    yup::String params = "/c " + batFile.getFile().getFullPathName().quoted();

    yup::HeapBlock<char> commandLine (params.getNumBytesAsUTF8() + 1);
    strcpy (commandLine.get(), params.toRawUTF8());

    yup::File cmdExe = yup::File::getSpecialLocation (yup::File::windowsSystemDirectory).getChildFile ("cmd.exe");
    jassert (cmdExe.existsAsFile());

    if (! CreateProcessA (cmdExe.getFullPathName().toRawUTF8(), commandLine.get(), nullptr, nullptr, 
                          false, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
        return {};
    
    WaitForSingleObject (pi.hProcess, INFINITE);

    CloseHandle (pi.hProcess);
    CloseHandle (pi.hThread);
   #else
    cmd << " > " << tmpFile.getFile().getFullPathName().quoted();
    auto result = std::system (cmd.toRawUTF8());
    yup::ignoreUnused (result);
   #endif

    auto contents = tmpFile.getFile().loadFileAsString();
    return contents;
}

class FFmpegEncoderAudioFormat::Writer : public yup::AudioFormatWriter
{
public:
    Writer (std::unique_ptr<yup::OutputStream>& destStream,
            const yup::String& formatName,
            const yup::File& exe, int vbr, int cbr,
            const yup::WriteOptions& options)
        : AudioFormatWriter (destStream.release(), formatName,
                             options.getSampleRate(),
                             (unsigned int) options.getNumChannels(),
                             (unsigned int) options.getBitsPerSample()),
          vbrLevel (vbr), cbrBitrate (cbr), ffmpeg (exe)
    {
        metadata.addUnorderedMap (options.getMetadataValues());

        if (auto out = std::unique_ptr<yup::OutputStream> (tempWav.getFile().createOutputStream()))
            writer = yup::WavAudioFormat().createWriterFor (out, options);
    }

    ~Writer() override
    {
        if (writer != nullptr)
        {
            writer = nullptr;
            
            if (! convertToMP3())
                convertToMP3();
        }
    }
    
    bool write (const int** samplesToWrite, int numSamples) override
    {
        return writer != nullptr && writer->write (samplesToWrite, numSamples);
    }
    
private:
    int vbrLevel, cbrBitrate;
    yup::TemporaryFile tempWav { ".wav" };
    std::unique_ptr<AudioFormatWriter> writer;

    bool runFFmpegChildProcess (const yup::TemporaryFile& tempMP3, const yup::StringArray& processArgs) const
    {
        yup::ChildProcess cp;

        DBG(processArgs.joinIntoString (" "));

        [[maybe_unused]] auto output = readOutputFromSystem (processArgs.joinIntoString (" "));
        DBG(output);
        
        return tempMP3.getFile().getSize() > 0;
    }
    
    bool convertToMP3() const
    {
        yup::TemporaryFile tempMP3 (".mp3");

        yup::StringArray args;

        args.add (ffmpeg.getFullPathName().quoted());
        args.add ("-i");
        args.add (tempWav.getFile().getFullPathName().quoted());
        args.add ("-codec:a");
        args.add ("libmp3lame");

        if (cbrBitrate == 0)
        {
            args.add ("-q:a");
            args.add (yup::String (vbrLevel));
        }
        else
        {
            args.add ("-b:a");
            args.add (yup::String::formatted ("%dk", cbrBitrate));
        }

        addMetadataArg (args, "id3title",       "title");
        addMetadataArg (args, "id3artist",      "artist");
        addMetadataArg (args, "id3album",       "album");
        addMetadataArg (args, "id3comment",     "comment");
        addMetadataArg (args, "id3date",        "date");
        addMetadataArg (args, "id3genre",       "genre");
        addMetadataArg (args, "id3trackNumber", "track");

        args.add (tempMP3.getFile().getFullPathName().quoted());

        if (runFFmpegChildProcess (tempMP3, args))
        {
            yup::FileInputStream fis (tempMP3.getFile());

            if (fis.openedOk() && output->writeFromInputStream (fis, -1) > 0)
            {
                output->flush();
                return true;
            }
        }
        
        return false;
    }

    void addMetadataArg (yup::StringArray& args, const char* key, const char* ffmpegFlag) const
    {
        auto value = metadata.getValue (key, {});

        if (value.isNotEmpty())
        {
            args.add ("-metadata");
            args.add (yup::String (ffmpegFlag) + "=" + value.quoted());
        }
    }

    const yup::File ffmpeg;
    yup::StringPairArray metadata;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Writer)
};

//==============================================================================
FFmpegEncoderAudioFormat::FFmpegEncoderAudioFormat (const yup::File& ffmpeg)
    : AudioFormat ("MP3 file", ".mp3"), ffmpegExe (ffmpeg)
{
}

FFmpegEncoderAudioFormat::~FFmpegEncoderAudioFormat()
{
}

bool FFmpegEncoderAudioFormat::canHandleFile (const yup::File&)
{
    return false;
}

yup::Array<int> FFmpegEncoderAudioFormat::getPossibleSampleRates()
{
    return { 32000, 44100, 48000 };
}

yup::Array<int> FFmpegEncoderAudioFormat::getPossibleBitDepths()
{
    return { 16 };
}

bool FFmpegEncoderAudioFormat::canDoStereo()      { return true; }
bool FFmpegEncoderAudioFormat::canDoMono()        { return true; }
bool FFmpegEncoderAudioFormat::isCompressed()     { return true; }

yup::StringArray FFmpegEncoderAudioFormat::getQualityOptions()
{
    static const char* vbrOptions[] = { "VBR quality 0 (best)", "VBR quality 1", "VBR quality 2", "VBR quality 3",
        "VBR quality 4 (normal)", "VBR quality 5", "VBR quality 6", "VBR quality 7",
        "VBR quality 8", "VBR quality 9 (smallest)", nullptr };

    yup::StringArray opts (vbrOptions);

    const int cbrRates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
    
    for (int i = 0; i < std::ssize (cbrRates); ++i)
        opts.add (yup::String (cbrRates[i]) + " Kb/s CBR");

    return opts;
}

yup::AudioFormatReader* FFmpegEncoderAudioFormat::createReaderFor (yup::InputStream*, const bool)
{
    return nullptr;
}

std::unique_ptr<yup::AudioFormatWriter> FFmpegEncoderAudioFormat::createWriterFor (std::unique_ptr<yup::OutputStream>& streamToWriteTo,
                                                                                    const yup::WriteOptions& options)
{
    if (! streamToWriteTo)
        return {};

    int vbr = 4;
    int cbr = 0;
    
    const yup::String qual (getQualityOptions() [options.getQualityOptionIndex()]);

    if (qual.contains ("VBR"))
        vbr = qual.retainCharacters ("0123456789").getIntValue();
    else
        cbr = qual.getIntValue();

    yup::StringPairArray metadata;
    metadata.addUnorderedMap (options.getMetadataValues());

    return std::make_unique<Writer> (streamToWriteTo,
                                     getFormatName(), ffmpegExe, vbr, cbr,
                                     options);
}

#endif

} // namespace tracktion::inline engine
