/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class CurveEditor;

// Vertical components will ignore mouse events if they occur over a comp which
// inherits from this class..
struct TransparentToMouseInTrackItemComp {};


struct CurvePoint
{
    CurvePoint() = default;
    CurvePoint (EditPosition t, float val) : time (t), value (val) {}

    EditPosition time;
    float value = 0;
};

//==============================================================================
class CurveEditorPoint  : public Selectable
{
public:
    CurveEditorPoint() noexcept {}
    CurveEditorPoint (int i, CurveEditor* ed)  : index (i), editor (ed) {}
    ~CurveEditorPoint() override;

    void selectionStatusChanged (bool isNowSelected) override;

    static bool arePointsConsecutive (const SelectableList&);
    static bool arePointsOnSameCurve (const SelectableList&);

    static EditTimeRange getPointEditPositionRange (const SelectableList&);

    int index = 0;
    CurveEditor* editor = nullptr;
};


//==============================================================================
class CurveEditor  : public yup::Component,
                     public SelectableListener,
                     public yup::ChangeListener,
                     public yup::TooltipClient,
                     public MouseHoverDetector,
                     public TransparentToMouseInTrackItemComp
{
public:
    CurveEditor (Edit&, SelectionManager&);
    ~CurveEditor() override;

    void paint (yup::Graphics&) override;
    bool hitTest (int x, int y) override;
    void visibilityChanged() override;

    void mouseDown (const yup::MouseEvent&) override;
    void mouseUp (const yup::MouseEvent&) override;
    void mouseDrag (const yup::MouseEvent&) override;
    void mouseMove (const yup::MouseEvent&) override;
    void mouseEnter (const yup::MouseEvent&) override;
    void mouseExit (const yup::MouseEvent&) override;
    void mouseDoubleClick (const yup::MouseEvent&) override;
    void mouseHovered (int mouseX, int mouseY) override;
    void mouseMovedAfterHover();

    yup::MouseCursor getMouseCursor();

    void setTimes (EditPosition leftTime, EditPosition rightTime);
    EditTimeRange getTimes() const;

    bool getDrawPointInsertionIndicator() const;
    void setDrawPointInsertionIndicator (bool);

    virtual float timeToX (EditPosition) const;
    virtual EditPosition xToTime (double x) const;
    float valueToY (float val) const;
    float yToValue (double y) const;
    yup::Point<float> getPosition (CurvePoint) const;
    yup::Point<float> getPosition (int index);

    void selectableObjectChanged (Selectable*) override;
    void selectableObjectAboutToBeDeleted (Selectable*) override;

    void changeListenerCallback (yup::ChangeBroadcaster*) override;
    void updateLineThickness();

    Edit& getEdit() const;
    virtual Track* getTrack()      { return {}; }

    virtual EditPosition snapTime (EditPosition, yup::ModifierKeys);

    virtual float getValueAt (EditPosition) = 0;
    virtual EditPosition getPointPosition (int idx) = 0;
    virtual float getPointValue (int idx) = 0;
    virtual float getPointCurve (int idx) = 0;
    virtual void removePoint (int index) = 0;
    virtual int addPoint (EditPosition time, float value, float curve) = 0;
    virtual int getNumPoints() = 0;
    virtual CurvePoint getBezierHandle (int idx) = 0;
    virtual CurvePoint getBezierPoint (int idx) = 0;
    virtual int nextIndexAfter (EditPosition) = 0;
    virtual void getBezierEnds (int index, double& x1out, float& y1out, double& x2out, float& y2out) = 0;
    virtual std::pair<CurvePoint, CurvePoint> getBezierEnds (int index) = 0;
    virtual int movePoint (int index, EditPosition newTime, float newValue, bool removeInterveningPoints) = 0;
    virtual void setValueWhenNoPoints (float value) = 0;
    virtual CurveEditorPoint* createPoint (int idx) = 0;
    virtual int curvePoint (int index, float newCurve) = 0;
    virtual yup::String getCurveName() = 0;
    virtual int getCurveNameOffset() = 0;
    virtual Selectable* getItem() = 0;
    virtual bool isShowingCurve() const = 0;
    virtual void updateFromTrack() = 0;

    virtual yup::Color getCurrentLineColor() = 0;
    virtual yup::Color getCurrentFillColor();
    virtual yup::Color getDefaultLineColor() const = 0;
    virtual yup::Color getSelectedLineColor() const = 0;
    virtual yup::Color getBackgroundColor() const = 0;
    virtual yup::Color getCurveNameTextBackgroundColor() const = 0;
    virtual yup::Color getPointOutlineColor() const = 0;

    virtual float getLineThickness (bool mouseOverOrDragging, bool curveOrPointsSelected) const;
    virtual void drawLine (yup::Graphics&, yup::Path, float lineThickness);

    void selectPoint (int pointIdx, bool addToSelection);

    virtual EditPosition toEditPosition (const TimePosition& t);
    virtual yup::Range<float> getParameterRange() const = 0;

protected:
    virtual bool fillPath() const { return true; }
    virtual bool isBeats() const { return false; }
    EditPosition fromUnderlying (double u);
    void updatePointUnderMouse (yup::Point<float>);
    virtual void showBubbleForPointUnderMouse() = 0;
    virtual void hideBubble() = 0;

    virtual void nonRealTimeDragStart() {}
    virtual void nonRealTimeDragEnd()   {}

    virtual bool canSelectItem() const { return true; }

    bool isPointSelected (int idx);
    bool areAnyPointsSelected();
    CurveEditorPoint* getSelectedPoint (int);

    static constexpr const float pointRadius = 5.0f;

    Edit& edit;
    juce::UndoManager& undoManager;
    SelectionManager& selectionManager;
    EditPosition leftTime, rightTime;
    int firstIndexOnScreen = 0;
    int pointUnderMouse = -1, pointBeingMoved = -1;
    int curveUnderMouse = -1, lineUnderMouse = -1;
    bool dragged = false, movingAllPoints = false;
    EditPosition mouseDownTime;
    float mouseDownValue = 0;
    bool isCurveSelected = false;
    float mouseDownCurve = 0;
    float point1 = 0, point2 = 0;
    bool realTimeDrag = true;
    float defaultCurve = 0;
    float lineThickness = 1.0f;
    bool drawPointInsertionIndicator = false;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CurveEditor)
};

} // namespace tracktion::inline engine
