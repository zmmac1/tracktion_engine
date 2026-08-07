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
    Manages the LAME location property and an AudioFormat if the LAME encoder is
    provided.
*/
class LAMEManager
{
public:
    //==============================================================================
    /** Returns the current LAME file. */
    static yup::File getLameEncoderExe();

    /** Returns the current FFmpeg file. */
    static yup::File getFFmpegExe();

    /** Returns the LAME shared library to load when TRACKTION_ENABLE_LIBLAME is set.
        This can be a full path or a bare library name, in which case the platform's
        usual library search path is used.
        @see LibLameEncoderAudioFormat
    */
    static yup::String getLameSharedLibrary();

    /** Add the LAMEAudioFormat to the AudioFileFormatManager */
    static void registerAudioFormat (AudioFileFormatManager&);

    /** Returns true if a valid LAME/FFmpeg file is found. */
    static bool lameIsAvailable();

private:
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LAMEManager)
};

} // namespace tracktion::inline engine
