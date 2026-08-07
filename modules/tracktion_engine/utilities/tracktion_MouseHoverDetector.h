/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class MouseHoverDetector
{
public:
    MouseHoverDetector (int hoverTimeMillisecs_ = 400)
        : hoverTimeMillisecs (hoverTimeMillisecs_)
    {
        internalTimer.owner = this;
    }

    virtual ~MouseHoverDetector()
    {
        setHoverComponent (nullptr, false);
    }

    void setHoverTimeMillisecs (int newTimeInMillisecs)
    {
        hoverTimeMillisecs = newTimeInMillisecs;
    }

    void setHoverComponent (yup::Component* newSourceComponent,
                            bool wantsEventsForAllNestedChildComponents)
    {
        if (source != newSourceComponent)
        {
            internalTimer.stopTimer();
            hasJustHovered = false;
            wantsHoversForAllNestedChildComponents = wantsEventsForAllNestedChildComponents;

            if (source != nullptr)
                source->removeMouseListener (&internalTimer);

            source = newSourceComponent;

            if (newSourceComponent != nullptr)
                newSourceComponent->addMouseListener (&internalTimer);
        }
    }

    void hoverTimerCallback()
    {
        internalTimer.stopTimer();

        if (source != nullptr)
        {
            auto pos = source->getMouseXYRelative();

            if (source->reallyContains (pos, wantsHoversForAllNestedChildComponents))
            {
                hasJustHovered = true;
                mouseHovered (pos.getX(), pos.getY());
            }
        }
    }

    void checkJustHoveredCallback()
    {
        if (hasJustHovered)
        {
            hasJustHovered = false;
            mouseMovedAfterHover();
        }
    }

protected:
    virtual void mouseHovered (int mouseX, int mouseY) = 0;
    virtual void mouseMovedAfterHover() = 0;

private:
    //==============================================================================
    class HoverDetectorInternal  : public yup::MouseListener,
                                   public yup::Timer
    {
    public:
        HoverDetectorInternal() = default;

        MouseHoverDetector* owner = nullptr;
        yup::Point<int> lastPos;

        void timerCallback() override
        {
            owner->hoverTimerCallback();
        }

        void mouseEnter (const yup::MouseEvent&) override
        {
            stopTimer();
            owner->checkJustHoveredCallback();
        }

        void mouseExit (const yup::MouseEvent&) override
        {
            stopTimer();
            owner->checkJustHoveredCallback();
        }

        void mouseDown (const yup::MouseEvent&) override
        {
            stopTimer();
            owner->checkJustHoveredCallback();
        }

        void mouseUp (const yup::MouseEvent&) override
        {
            stopTimer();
            owner->checkJustHoveredCallback();
        }

        void mouseMove (const yup::MouseEvent& e) override
        {
            if (lastPos != e.getPosition()) // to avoid fake mouse-moves setting it off
            {
                lastPos = e.getPosition();

                if (owner->source != nullptr)
                    startTimer (owner->hoverTimeMillisecs);

                owner->checkJustHoveredCallback();
            }
        }

        void mouseWheel (const yup::MouseEvent&, const yup::MouseWheelData&) override
        {
            stopTimer();
            owner->checkJustHoveredCallback();
        }

        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HoverDetectorInternal)
    };

    HoverDetectorInternal internalTimer;
    yup::Component* source = nullptr;
    int hoverTimeMillisecs;
    bool hasJustHovered = false;
    bool wantsHoversForAllNestedChildComponents = false;

    YUP_DECLARE_NON_COPYABLE (MouseHoverDetector)
};

} // namespace tracktion::inline engine
