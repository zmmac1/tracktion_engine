/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine
{

//==============================================================================
struct DiscreteLabelledParameter  : public AutomatableParameter
{
    DiscreteLabelledParameter (const yup::String& xmlTag,
                               const yup::String& name,
                               AutomatableEditItem& owner,
                               yup::Range<float> valueRangeToUse,
                               int numStatesToUse = 0,
                               yup::StringArray labelsToUse = {})
        : AutomatableParameter (xmlTag, name, owner, valueRangeToUse),
          numStates (numStatesToUse), labels (labelsToUse)
    {
        jassert (labels.isEmpty() || labels.size() == numStates);
    }

    ~DiscreteLabelledParameter() override
    {
        notifyListenersOfDeletion();
    }

    bool isDiscrete() const override             { return numStates != 0; }
    int getNumberOfStates() const override       { return numStates; }

    float getValueForState (int i) const override
    {
        if (numStates == 0)
            return 0.0;

        return yup::jmap ((float) i, 0.0f, float (numStates - 1), valueRange.start, valueRange.end);
    }

    int getStateForValue (float value) const override
    {
        if (numStates == 0)
            return 0;

        return yup::roundToInt (yup::jmap (value, valueRange.start, valueRange.end, 0.0f, float (numStates - 1)));
    }

    bool hasLabels()  const override                   { return labels.size() > 0; }
    yup::StringArray getAllLabels() const override    { return labels; }

    yup::String getLabelForValue (float val) const override
    {
        const int s = getStateForValue (val);

        return yup::isPositiveAndBelow (s, getNumberOfStates()) ? labels[s] : yup::String();
    }

    float snapToState (float val) const override
    {
        return getValueForState (yup::jlimit (0, getNumberOfStates() - 1, yup::roundToInt (val)));
    }

private:
    const int numStates = 0;
    const yup::StringArray labels;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiscreteLabelledParameter)
};

//==============================================================================
struct SuffixedParameter    : public AutomatableParameter
{
    SuffixedParameter (const yup::String& xmlTag, const yup::String& name,
                       AutomatableEditItem& owner, yup::NormalisableRange<float> valueRangeToUse,
                       yup::String suffixToUse = {})
        : AutomatableParameter (xmlTag, name, owner, valueRangeToUse),
          suffix (suffixToUse)
    {
    }

    ~SuffixedParameter() override
    {
        notifyListenersOfDeletion();
    }

    yup::String valueToString (float value) override
    {
        if (valueRange.interval == 1.0f)
            return yup::String (yup::roundToInt (value));

        return AutomatableParameter::valueToString (value);
    }

    yup::String getLabel() override           { return suffix; }

    const yup::String suffix;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuffixedParameter)
};


//==============================================================================
/// A helper class that creates an AutomatableParameter and links it to a
/// CachedValue, to make it easier to create and manage parameters whose values
/// are stored as a property in a plugin's ValueTree state.
struct ParameterWithStateValue
{
    ParameterWithStateValue() = default;

    ParameterWithStateValue (Plugin&,
                             const yup::String& paramID,
                             const yup::Identifier& propertyName,
                             const yup::String& description,
                             float defaultValue,
                             yup::NormalisableRange<float> valueRange,
                             std::function<yup::String(float)> valueToStringFunction = {},
                             std::function<float(const yup::String&)> stringToValueFunction = {});

    ~ParameterWithStateValue();
    ParameterWithStateValue (ParameterWithStateValue&&) = default;
    ParameterWithStateValue& operator= (ParameterWithStateValue&&) = default;

    AutomatableParameter& getParameter()            { return *parameter; }
    juce::CachedValue<float>& getCachedValue()      { return *value; }

    /// Returns the current value from the AutomatableParameter
    float getCurrentValue() const                   { return parameter->getCurrentValue(); }
    /// Sets a new value via the AutomatableParameter
    void setParameter (float newValue, yup::NotificationType);

    /// Releases the parameter and CachedValue
    void reset();

    /// Copies the matching property from the given ValueTree to this parameter
    void setFromValueTree (const yup::ValueTree&);

    AutomatableParameter::Ptr parameter;
    std::unique_ptr<juce::CachedValue<float>> value;
};


} // namespace tracktion::inline engine
