/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com
*/


namespace tracktion::inline engine {

static int numScreenSaverDefeaters = 0;

ScreenSaverDefeater::ScreenSaverDefeater()
{
    if (yup::Desktop::getInstance()->isHeadless())
        return;

    TRACKTION_ASSERT_MESSAGE_THREAD
    ++numScreenSaverDefeaters;
    yup::Desktop::setScreenSaverEnabled (numScreenSaverDefeaters == 0);
}

ScreenSaverDefeater::~ScreenSaverDefeater()
{
    if (yup::Desktop::getInstance()->isHeadless())
        return;

    TRACKTION_ASSERT_MESSAGE_THREAD
    --numScreenSaverDefeaters;
    jassert (numScreenSaverDefeaters >= 0);
    yup::Desktop::setScreenSaverEnabled (numScreenSaverDefeaters == 0);
}


} // namespace tracktion::inline engine
