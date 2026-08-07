/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

struct ScopedCpuMeter
{
    ScopedCpuMeter (std::atomic<double>& valueToUpdate_, double filterAmount_) noexcept
        : valueToUpdate (valueToUpdate_), filterAmount (filterAmount_)
    {
    }

    ~ScopedCpuMeter() noexcept
    {
        const double msTaken = yup::Time::getMillisecondCounterHiRes() - callbackStartTime;
        const auto lastValue = valueToUpdate.load (std::memory_order_acquire);
        valueToUpdate.store (lastValue + filterAmount * (msTaken - lastValue),
                             std::memory_order_release);
    }

private:
    std::atomic<double>& valueToUpdate;
    const double filterAmount, callbackStartTime { yup::Time::getMillisecondCounterHiRes() };
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedCpuMeter)
};

//==============================================================================
struct StopwatchTimer
{
    StopwatchTimer() noexcept    : time (yup::Time::getCurrentTime()) {}

    yup::RelativeTime getTime() const noexcept  { return yup::Time::getCurrentTime() - time; }
    yup::String getDescription() const          { return getTime().getDescription(); }
    double getSeconds() const noexcept           { return getTime().inSeconds(); }

private:
    const yup::Time time;
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StopwatchTimer)
};

//==============================================================================
#if TRACKTION_CHECK_FOR_SLOW_RENDERING
 struct RealtimeCheck
 {
    RealtimeCheck (const char* f, int l, double maxMillisecs) noexcept
        : start (yup::Time::getMillisecondCounterHiRes()),
          end (start + maxMillisecs),
          file (f), line (l)
    {
    }

    ~RealtimeCheck() noexcept
    {
        const double now = yup::Time::getMillisecondCounterHiRes();

        if (now > end)
            yup::Logger::outputDebugString (yup::String (now - start) + " " + file + ":" + yup::String (line));
    }

    double start, end;
    const char* file;
    int line;
 };

 #define SCOPED_REALTIME_CHECK           const RealtimeCheck JUCE_JOIN_MACRO(__realtimeCheck, __LINE__) (__FILE__, __LINE__, 1.5);
 #define SCOPED_REALTIME_CHECK_LONGER    const RealtimeCheck JUCE_JOIN_MACRO(__realtimeCheck, __LINE__) (__FILE__, __LINE__, 200 / 44.1);
#else
 #define SCOPED_REALTIME_CHECK
 #define SCOPED_REALTIME_CHECK_LONGER
#endif

} // namespace tracktion::inline engine
