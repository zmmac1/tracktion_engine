/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** */
class RandomModifier    : public Modifier,
                          private ValueTreeAllEventListener
{
public:
    //==============================================================================
    RandomModifier (Edit&, const yup::ValueTree&);
    ~RandomModifier() override;

    using Ptr = yup::ReferenceCountedObjectPtr<RandomModifier>;
    using Array = yup::ReferenceCountedArray<RandomModifier>;

    using Modifier::initialise;
    void initialise() override;
    yup::String getName() const override               { return TRANS("Random Modifier"); }

    //==============================================================================
    /** Returns the current value of the LFO.
        N.B. this may be greater than +-1 if a suitable offset has been applied.
    */
    float getCurrentValue() override;

    /** Returns the current phase between 0 & 1. */
    float getCurrentPhase() const noexcept;

    AutomatableParameter::ModifierAssignment* createAssignment (const yup::ValueTree&) override;

    ProcessingPosition getProcessingPosition() override { return ProcessingPosition::preFX; }
    void applyToBuffer (const PluginRenderContext&) override;

    //==============================================================================
    struct Assignment : public AutomatableParameter::ModifierAssignment
    {
        Assignment (const yup::ValueTree&, const RandomModifier&);

        bool isForModifierSource (const ModifierSource&) const override;
        yup::ReferenceCountedObjectPtr<RandomModifier> getRandomModifier() const;

        const EditItemID randomModifierID;

        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Assignment)
    };

    //==============================================================================
    yup::String getSelectableDescription() override    { return getName(); }

    //==============================================================================
    enum Type
    {
        random  = 0,
        noise   = 1
    };

    static yup::StringArray getTypeNames();

    juce::CachedValue<float> type, shape, syncType, rate, rateType, depth, stepDepth, smooth, bipolar;

    AutomatableParameter::Ptr typeParam, shapeParam, syncTypeParam, rateParam, rateTypeParam, depthParam, stepDepthParam, smoothParam, bipolarParam;

private:
    struct RandomModifierTimer;
    std::unique_ptr<RandomModifierTimer> modifierTimer;

    yup::Random rand;
    LambdaTimer changedTimer;
    std::atomic<float> currentPhase { 1.0f }, currentValue { 0.0f };
    float previousRandom = 0.0f, currentRandom = 0.0f, randomDifference = 0.0f;

    void setPhase (float newPhase);
    void valueTreeChanged() override;
};

} // namespace tracktion::inline engine
