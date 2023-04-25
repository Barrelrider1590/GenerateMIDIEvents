/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GenerateMIDIEventsAudioProcessor::GenerateMIDIEventsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : m_midiChannel(10), m_startTime(juce::Time::getMillisecondCounterHiRes()),
       AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

}

GenerateMIDIEventsAudioProcessor::~GenerateMIDIEventsAudioProcessor()
{
}

//==============================================================================
const juce::String GenerateMIDIEventsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GenerateMIDIEventsAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GenerateMIDIEventsAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GenerateMIDIEventsAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GenerateMIDIEventsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GenerateMIDIEventsAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GenerateMIDIEventsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GenerateMIDIEventsAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GenerateMIDIEventsAudioProcessor::getProgramName (int index)
{
    return {};
}

void GenerateMIDIEventsAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GenerateMIDIEventsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void GenerateMIDIEventsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GenerateMIDIEventsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GenerateMIDIEventsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool GenerateMIDIEventsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GenerateMIDIEventsAudioProcessor::createEditor()
{
    return new GenerateMIDIEventsAudioProcessorEditor (*this);
}

//==============================================================================
void GenerateMIDIEventsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GenerateMIDIEventsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GenerateMIDIEventsAudioProcessor();
}

void GenerateMIDIEventsAudioProcessor::setNoteNumber(int noteNumber)
{
    auto message{juce::MidiMessage::noteOn(m_midiChannel, noteNumber, (juce::uint8) 100)};
    message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * .001 - m_startTime);
    addMessageToList(message);
}

void GenerateMIDIEventsAudioProcessor::addMessageToList(const juce::MidiMessage& message)
{
    auto time{ message.getTimeStamp() };

    int hours{ static_cast<int>(time / 3600.0) % 24 };
    int minutes{ static_cast<int>(time / 60.0) % 60 };
    int seconds{ static_cast<int>(time) % 60 };
    int millis{ static_cast<int>(time * 1000.0) % 1000 };
    
    juce::String timeCode{ juce::String::formatted("%02d:%02d:%02d:%03d",
                                                   hours, minutes, seconds, millis) };

    logMessage(timeCode + " - " + getMidiMessageDescription(message));
}

juce::String GenerateMIDIEventsAudioProcessor::getMidiMessageDescription(const juce::MidiMessage& message)
{
    return message.getTextFromTextMetaEvent();
}

void GenerateMIDIEventsAudioProcessor::logMessage(const juce::String& message)
{

}