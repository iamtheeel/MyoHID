# MyoHID
Myoelectric Control of a Human Interface Device


![Video of System in Action](https://github.com/iamtheeel/MyoHID/blob/744d13a04d67df1fa4955e9585844b020ff10742/IMG_6636_finalRun_720p.mov)

The default method of interfacing with computational equipment is the keyboard and mouse. These inputs can be challenging for people with even mild disabilities. If not used correctly, improper use can lead to nerve damage causing disability. 
We have captured electromyography data with low cost off the shelf hardware, processed the signals with the help of a machine learning algorithm, and then used that processed information to control a human interface device to operate a mouse for any common computer system.


The software is broken into three major categories. 
*	Data Acquisition (DAQ)
    -	arduinoDAQ.ino
    -	serialComms.cpp
    - serialComms.h
*	Signal Processing (MATLAB)
    -	The entry point
        - frontEnd.mlx
    -	Record data for analysis and training
        - recordDataBlocks.m
        - blockExtract.m
    -	Run Realtime capture of data from the DAQ, process data, and send to the HID
        - realTime.m
    -	Serial packets receive and parse, Circular buffer for Realtime
        - dataHandeler.m
    -	Feature Extraction
        - dataProc.m
        - calcFFT.m
    -	Data Analisis tools
        - plotFeatures.m
*	Human Interface Device (HID)
    -	HID.ino
    -	gestureMap.h
    -	serialComms.cpp
    -	serialComms.h
