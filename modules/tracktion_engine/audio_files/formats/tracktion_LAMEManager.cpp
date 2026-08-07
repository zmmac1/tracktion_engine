/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

#if JUCE_USE_LAME_AUDIO_FORMAT

//==============================================================================
yup::File LAMEManager::getLameEncoderExe()
{
   #if JUCE_WINDOWS
    auto defaultLame = yup::File::getSpecialLocation (yup::File::currentExecutableFile).getSiblingFile ("lame.exe");
   #elif YUP_MAC
    auto defaultLame = yup::File::getSpecialLocation (yup::File::currentExecutableFile).getSiblingFile ("../Resources/lame");
   #else
    yup::File defaultLame ("/usr/bin/lame");
   #endif

    return defaultLame;
}

yup::File LAMEManager::getFFmpegExe()
{
   #if JUCE_WINDOWS
    auto defaultFFmpeg = yup::File::getSpecialLocation (yup::File::currentExecutableFile).getSiblingFile ("ffmpeg.exe");
   #elif YUP_MAC
    auto defaultFFmpeg = yup::File::getSpecialLocation (yup::File::currentExecutableFile).getSiblingFile ("../Resources/ffmpeg");
   #else
    yup::File defaultFFmpeg ("/usr/bin/ffmpeg");
   #endif

    return defaultFFmpeg;
}

yup::String LAMEManager::getLameSharedLibrary()
{
   #if TRACKTION_ENABLE_LIBLAME
    auto libraryName = LibLameEncoderAudioFormat::getDefaultLibraryName();
    auto exe = yup::File::getSpecialLocation (yup::File::currentExecutableFile);

    // Prefer a copy shipped with the app. On macOS, Frameworks is where a dylib is
    // meant to live as far as codesigning and notarisation are concerned, but check
    // Resources too since that's where the lame and ffmpeg executables go
   #if YUP_MAC
    const char* relativePaths[] = { "../Frameworks/", "../Resources/" };
   #else
    const char* relativePaths[] = { "" };
   #endif

    for (auto path : relativePaths)
        if (auto bundled = exe.getSiblingFile (path + libraryName); bundled.existsAsFile())
            return bundled.getFullPathName();

    // Otherwise let the platform's library search path find it
    return libraryName;
   #else
    return {};
   #endif
}

#if TRACKTION_ENABLE_LIBLAME
/** Opening the shared library isn't free and the render options ask whether LAME is
    available repeatedly, so the answer only gets worked out once.
*/
static bool isLameSharedLibraryAvailable()
{
    static const bool available = LibLameEncoderAudioFormat (LAMEManager::getLameSharedLibrary()).isLibraryLoaded();
    return available;
}
#endif

/** Adds lame encoders to the audio file format manager if necessary. */
void LAMEManager::registerAudioFormat (AudioFileFormatManager& affm)
{
    if (affm.getLameFormat() != nullptr || ! LAMEManager::lameIsAvailable())
        return;

   #if TRACKTION_ENABLE_LIBLAME
    // Preferred when it's there, as it encodes straight to the destination stream
    // rather than going via a temp file and a command line tool
    if (isLameSharedLibraryAvailable())
    {
        auto library = LAMEManager::getLameSharedLibrary();
        TRACKTION_LOG ("LAME: using shared library: " + library);

        affm.addLameFormat (std::make_unique<LibLameEncoderAudioFormat> (library),
                            std::make_unique<LibLameEncoderAudioFormat> (library));
        return;
    }
   #endif

   #if TRACKTION_ENABLE_FFMPEG
    TRACKTION_LOG ("LAME: using exe: " + LAMEManager::getFFmpegExe().getFullPathName());

    affm.addLameFormat (std::make_unique<FFmpegEncoderAudioFormat> (LAMEManager::getFFmpegExe()),
                        std::make_unique<FFmpegEncoderAudioFormat> (LAMEManager::getFFmpegExe()));
   #else
    TRACKTION_LOG ("LAME: using exe: " + LAMEManager::getLameEncoderExe().getFullPathName());

    affm.addLameFormat (std::make_unique<yup::LAMEEncoderAudioFormat> (LAMEManager::getLameEncoderExe()),
                        std::make_unique<yup::LAMEEncoderAudioFormat> (LAMEManager::getLameEncoderExe()));
   #endif
}

bool LAMEManager::lameIsAvailable()
{
   #if TRACKTION_ENABLE_LIBLAME
    if (isLameSharedLibraryAvailable())
        return true;
   #endif

   #if TRACKTION_ENABLE_FFMPEG
    auto ffmpeg = getFFmpegExe();
    return ffmpeg.exists() && ffmpeg.getFileName().containsIgnoreCase ("ffmpeg");
   #else
    auto lameEnc = getLameEncoderExe();
    return lameEnc.exists() && lameEnc.getFileName().containsIgnoreCase ("lame");
   #endif
}

//==============================================================================
#else

yup::File LAMEManager::getLameEncoderExe()     { return {}; }
yup::String LAMEManager::getLameSharedLibrary() { return {}; }
void LAMEManager::registerAudioFormat (AudioFileFormatManager&) {}
bool LAMEManager::lameIsAvailable()             { return false; }

#endif

} // namespace tracktion::inline engine
