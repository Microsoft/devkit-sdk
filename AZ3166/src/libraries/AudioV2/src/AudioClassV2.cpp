// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "AudioClassV2.h"
#include <stdint.h>
#include <stdlib.h>

#include "nau88c10.h"
#include "stm32f4xx_hal.h"
#include "stm32412g_discovery.h"
#include "stm32412g_discovery_audio.h"
#include "Arduino.h"

static uint32_t _sampleRate;
static uint16_t _sampleBitDepth;
static uint8_t _channels;
static uint8_t _durationInSeconds;

static char _play_buffer[AUDIO_CHUNK_SIZE];
static char _record_buffer[AUDIO_CHUNK_SIZE];

static char * _wavFile;
static char * _recordCursor;
static char * _playCursor;
static int _audioFileSize;

static volatile char _flag = 0;
static AUDIO_STATE_TypeDef _audioState;

static callbackFunc audioCallbackFptr = NULL;
static callbackFunc recordCallbackFptr = NULL;

AudioClass::AudioClass()
{
    format(DEFAULT_SAMPLE_RATE, DEFAULT_BITS_PER_SAMPLE);
}

/* 
 * @brief Configure the audio data format
*/
void AudioClass::format(unsigned int sampleRate, unsigned short sampleBitLength)
{
    _sampleRate = sampleRate;
    _sampleBitDepth = sampleBitLength;
    _channels = STEREO;

    // Currently we ONLY support 16 bit depth audio sample
    unsigned short bitDepth;
    if (sampleBitLength == 16)
    {
        bitDepth = I2S_DATAFORMAT_16B;
    }
    else if (sampleBitLength == 24)
    {
        // TODO
        bitDepth = I2S_DATAFORMAT_24B;
    }
    else if (sampleBitLength == 32)
    {
        // TODO
        bitDepth = I2S_DATAFORMAT_32B;
    }
    else
    {
        bitDepth = I2S_DATAFORMAT_16B;
    }

    BSP_AUDIO_IN_OUT_Init(OUTPUT_DEVICE_AUTO, bitDepth, sampleRate);

    // Set audio initialization state
    _audioState = AUDIO_STATE_INIT;
}

int AudioClass::startRecord(callbackFunc func)
{
    _wavFile = NULL;
    recordCallbackFptr = func;
    _audioState = AUDIO_STATE_RECORDING;
    memset(_play_buffer, 0x0, AUDIO_CHUNK_SIZE);
    BSP_AUDIO_In_Out_Transfer((uint16_t*)_play_buffer, (uint16_t*)_record_buffer, AUDIO_CHUNK_SIZE/2);

    return AUDIO_OK;
}

int AudioClass::startRecord(char* audioBuffer, int size)
{
    if (audioBuffer == NULL || size < WAVE_HEADER_SIZE) {
        // TODO: log error
        return -1;
    }

    recordCallbackFptr = NULL;
    _wavFile = audioBuffer;
    _audioFileSize = size;
    _recordCursor = _wavFile + WAVE_HEADER_SIZE;

    _audioState = AUDIO_STATE_RECORDING;
    memset(_play_buffer, 0x0, AUDIO_CHUNK_SIZE);
    BSP_AUDIO_In_Out_Transfer((uint16_t*)_play_buffer, (uint16_t*)_record_buffer, AUDIO_CHUNK_SIZE/2);

    return AUDIO_OK;
}

int AudioClass::getCurrentSize()
{
    if ((_audioState == AUDIO_STATE_RECORDING || _audioState == AUDIO_STATE_RECORDING_FINISH) && _wavFile != NULL)
    {
        return _recordCursor - _wavFile;
    }
    if ((_audioState == AUDIO_STATE_PLAYING || _audioState == AUDIO_STATE_PLAYING_FINISH) && _wavFile != NULL)
    {
        return _playCursor - _wavFile;
    }

    return 0;
}

int AudioClass::writeToPlayBuffer(char* buffer, int length)
{
    if (buffer == NULL || length < 0)
    {
        return -1;
    }

    int copySize = length;
    if (length > AUDIO_CHUNK_SIZE)
    {
        copySize = AUDIO_CHUNK_SIZE;
    }

    memcpy((char*)_play_buffer, buffer, copySize);
    return copySize;
}

int AudioClass::readFromRecordBuffer(char* buffer, int length)
{
    if (buffer == NULL || length < 0)
    {
        return -1;
    }

    int copySize = length;
    if (length > AUDIO_CHUNK_SIZE * 2)
    {
        copySize = AUDIO_CHUNK_SIZE * 2;
    }

    memcpy(buffer, _record_buffer, copySize);
    return copySize;
}

int AudioClass::startPlay(callbackFunc func)
{
    _wavFile = NULL;
    audioCallbackFptr = func;
    if (audioCallbackFptr != NULL)
    {
        // Write audio binary to Tx buffer
        audioCallbackFptr();
    }

    // Start the audio player
    _audioState = AUDIO_STATE_PLAYING;

    // Set the start pointer of the PCM raw data.
    uint16_t * pStart = (uint16_t *)(_play_buffer + WAVE_HEADER_SIZE);

    // Set the total number of data to be played (count in half-word)
    uint32_t playSize = (uint32_t)(AUDIO_CHUNK_SIZE - WAVE_HEADER_SIZE) / STEREO;

    BSP_AUDIO_OUT_Play(pStart, playSize);

    return AUDIO_OK;
}

int AudioClass::startPlay(char* audioBuffer, int size)
{
    if (audioBuffer == NULL || size < WAVE_HEADER_SIZE + AUDIO_CHUNK_SIZE)
    {
        return AUDIO_ERROR;
    }

    audioCallbackFptr = NULL;
    _wavFile = audioBuffer;
    _audioFileSize = size;
    _playCursor = _wavFile + WAVE_HEADER_SIZE;

    _audioState = AUDIO_STATE_PLAYING;
    BSP_AUDIO_OUT_Play((uint16_t *)_playCursor, AUDIO_CHUNK_SIZE / STEREO);
    _playCursor += AUDIO_CHUNK_SIZE;

    return AUDIO_OK;
}

/*
 * @brief stop audio data transmition
*/
void AudioClass::stop()
{
    if (_audioState == AUDIO_STATE_RECORDING && _wavFile != NULL)
    {
        int currentSize = _recordCursor - _wavFile;
    
        // write wave header for this audio file
        WaveHeader hdr;
        genericWAVHeader(&hdr, currentSize - WAVE_HEADER_SIZE, _sampleRate, _sampleBitDepth, _channels);
        memcpy(_wavFile, &hdr, sizeof(WaveHeader));
    }

    if (_audioState == AUDIO_STATE_RECORDING)
    {
        _audioState = AUDIO_STATE_RECORDING_FINISH;
        BSP_AUDIO_STOP();
    }

    if (_audioState == AUDIO_STATE_PLAYING)
    {
        _audioState = AUDIO_STATE_PLAYING_FINISH;
        BSP_AUDIO_STOP();
    }
}

int AudioClass::getAudioState()
{
    return _audioState;
}

/*
 * @brief compose the WAVE header according to the raw data size
 */
void AudioClass::genericWAVHeader(WaveHeader *hdr, int pcmDataSize, uint32_t sampleRate, uint16_t sampleBitDepth, uint8_t channels)
{
    if (hdr == NULL)
    {
        return;
    }

    memcpy(&hdr->RIFF_marker, "RIFF", 4);
    memcpy(&hdr->filetype_header, "WAVE", 4);
    memcpy(&hdr->format_marker, "fmt ", 4);
    hdr->data_header_length = 16;
    hdr->format_type = 1;
    hdr->number_of_channels = channels;
    hdr->sample_rate = sampleRate;
    hdr->bytes_per_second = sampleRate * channels * sampleBitDepth / 8;
    hdr->bytes_per_frame = channels * sampleBitDepth / 8;
    hdr->bits_per_sample = sampleBitDepth;
    hdr->file_size = pcmDataSize + 36;
    memcpy(&hdr->data_chunk_id, "data", 4);
    hdr->data_chunk_size = pcmDataSize;
}

int AudioClass::convertToMono(char* audioFile, int size, int sampleBitLength)
{
    if (sampleBitLength != 16 && sampleBitLength != 24 && sampleBitLength != 32)
    {
        // TODO: log error
        return -1;
    }

    if (audioFile == NULL || size < WAVE_HEADER_SIZE)
    {
        // TODO: log error
        return -1;
    }

    int bytesPerSample = sampleBitLength / 8;
    int curFileSize = 0;

    char *curReader = audioFile + WAVE_HEADER_SIZE + bytesPerSample * 2;
    char *curWriter = audioFile + WAVE_HEADER_SIZE + bytesPerSample;

    // Avoid using memcpy to improve performance
    if (sampleBitLength == 16)
    {
        while (curReader < audioFile + size)
        {
            *(uint16_t *)curWriter = *((uint16_t *)curReader);

            curWriter += bytesPerSample;
            curReader += bytesPerSample * 2;
        }
    }
    else if (sampleBitLength == 32)
    {
        while (curReader < audioFile + size)
        {
            *(uint32_t *)curWriter = *((uint32_t *)curReader);

            curWriter += bytesPerSample;
            curReader += bytesPerSample * 2;
        }
    }
    else
    {
        while (curReader < audioFile + size)
        {
            memcpy(curWriter, curReader, bytesPerSample);

            curWriter += bytesPerSample;
            curReader += bytesPerSample * 2;
        }
    }

    curFileSize = curWriter - audioFile;

    // re-calculate wave header since the raw data is re-sized from stereo to mono
    WaveHeader hdr;
    genericWAVHeader(&hdr, curFileSize - WAVE_HEADER_SIZE, _sampleRate, sampleBitLength, 1);
    memcpy(audioFile, &hdr, sizeof(WaveHeader));

    // clean up the remaining file
    //memset(curWriter, 0, audioFile + size - curWriter);

    return curFileSize;
}

/*------------------------------------------------------------------------------
       Callbacks implementation:
           the callbacks API are defined __weak in the stm32412g_discovery_audio.c file
           and their implementation should be done the user code if they are needed.
           Below some examples of callback implementations.
  ----------------------------------------------------------------------------*/
/**
  * @brief  Manages the full Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
    if (_audioState == AUDIO_STATE_RECORDING)
    {
        if (recordCallbackFptr != NULL)
        {
            recordCallbackFptr();
        }
        
        if (_wavFile != NULL)
        {
            if (_recordCursor + AUDIO_CHUNK_SIZE > _wavFile + _audioFileSize)
            {
                AudioClass::getInstance().stop();
                return;
            }
            
            memcpy(_recordCursor, _record_buffer, AUDIO_CHUNK_SIZE);
            _recordCursor += AUDIO_CHUNK_SIZE;
        }

        memset(_play_buffer, 0x0, AUDIO_CHUNK_SIZE);
        BSP_AUDIO_In_Out_Transfer((uint16_t*)_play_buffer, (uint16_t*)_record_buffer, AUDIO_CHUNK_SIZE/2);
    }
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    if (_audioState == AUDIO_STATE_PLAYING)
    {
        if (audioCallbackFptr != NULL)
        {
            audioCallbackFptr();
        }
        
        if (_wavFile != NULL)
        {    
            if (_playCursor + AUDIO_CHUNK_SIZE > _wavFile + _audioFileSize)
            {
                AudioClass::getInstance().stop();
                return;
            }
            
            memcpy(_play_buffer, _playCursor, AUDIO_CHUNK_SIZE);
            _playCursor += AUDIO_CHUNK_SIZE;
        }

        BSP_AUDIO_OUT_ChangeBuffer((uint16_t *)_play_buffer, DMA_MAX(AUDIO_CHUNK_SIZE/2));
    }
}


/**
  * @brief  Manages the DMA FIFO error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
    /* Display message on the LCD screen */
}