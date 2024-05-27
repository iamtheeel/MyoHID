%
% Capture/process/handel gestures
% Joshua Mehlman and James Swartz
%
% ENGR 845 Spring 2024
% Term Project:
%


% User vars
serialPort = "/dev/tty.us:
1:bmodem113401"; % UNIX format
%serialPort = "COM9" % Windows Format

% help BaudRate
% 115200 (default) | 50 | 75 | 110 | 134 | 150 | 200 | 300 | 600 | 1200 | 1800 | 2400 | 4800 | 9600 | 19200 | 38400 | 57600 | 230400
%serialSpeed =  230400;
serialSpeed = 2000000;
daqFreq = 300; % 
nCh = 4;

recordDataOnly = true;

HIDserialPort = "/dev/tty.usbmodem003806534"; % Berkeley UNIX format (is it consistant?)
%HIDserialPort = "com13"; % Berkeley UNIX format (is it consistant?)
HIDserialSpeed = 921600;
                   
mlModel_file = "4CH-5GES-QuadSVM_1";    %Name of the file withouth the ".mat"
%mlModel_file = "dummyModel";    %Name of the file withouth the ".mat"

mlModel_name = "trainedModel"; % Name of the model that is saved in the file

%250Hz is 50 packets to get to 200ms, 8 overlap for 32 ms
% 300Hz is 60 packets to get 200ms, 12 overlap for 40ms
nPacketsInBlock = 60; %1024;
%processingOverlap = 0.90; % What percent of the block to reuse 
%nPacketsForOverlap = floor(nPacketsInBlock*(1-processingOverlap));
% or set the nPackets directly e.x. for 500Hz:
nPacketsForOverlap = 12; % gives 40mS Englehart sugests 32mS 



% % movedf to UserData
% block = single([]); % Make an empty single presision array for the data
% packetCount = 0;


%Data
%format long

% Configs and one times
clear dataHand;
dataHand = dataHandeler.init(nPacketsInBlock, nPacketsForOverlap);



% Model
load(mlModel_file, mlModel_name)

%Timer
datetime.setDefaultFormats('default','yyyy-MM-dd_hhmmss');
dataAcqStartTime = datetime('now');

%start up the serial
clear serialDev;
serialDev = serialport(serialPort, serialSpeed);
configureTerminator(serialDev, "CR/LF");
serialDev.Terminator;

clear hidSerial;
hidSerial = serialport(HIDserialPort, HIDserialSpeed);

serialDev.UserData = struct("dataBlock",[],...
                            "nBytesPerPacket", 0, ...
                            "pktCount",0, ...
                            "pktsPerBlock", nPacketsInBlock, ...
                            "daqFreq", daqFreq, ...
                            "nReads", 0,...
                            "nCh", nCh, ...
                            "mlModel", trainedModel, ...
                            "aCount", 0, ... % for testing
                            "thisCmd", 0 ... % for tessting
                            );


% serialDev.flush(); % Make sure we start from 0
% configureCallback(serialDev,"terminator", @(src, evt) readSerialData(src, evt, dataHand))


pushbuttonPlot(serialDev, hidSerial, dataHand)

function pushbuttonPlot(sDev,hidD, dataH)
    f = figure;
    f.Position = [300 600, 300, 100];
    startDAQ = uicontrol(Position=[20 45 60 20]);
    startDAQ.String = 'Start DAQ';
    startDAQ.Callback = @(src, evt) startDAQBntPress(src, evt, sDev, hidD, dataH);

    function startDAQBntPress(src,evt, sDev, hidDe, dataHa)
        disp("Starting DAQ!");
        sDev.flush(); % Make sure we start from 0
        configureCallback(sDev,"terminator", @(src, evt) readSerialData(src, evt, hidDe, dataHa))
    end

    %
    % add a serial port list
    %

    %
    % Add a list of models to load
    %

    
    stopDAQ = uicontrol(Position=[20 20 60 20]);  % Position [left, bottom, width, height]
    stopDAQ.String = 'Stop DAQ';
    stopDAQ.Callback = @(src, evt) stopDAQBntPress(src, evt, sDev);

    function stopDAQBntPress(src,evt, sDev)
        configureCallback(sDev, "off");
        disp("DAQ Stopped!");
    end

end

%fuction buttiondown(

function readSerialData(src, evt, hidDev, dataHand)
    %tic;
    dataHand = dataHandeler.dataPacket_Min(src, dataHand);
    %dataHand = dataHandeler.dataPacket(src, dataHand);
    %toc
    % Configs and disposable data in dataHand
    %disp(['data: ', dataHand.packet ])

    % Persistant Data must be in src
    %disp(['Count: ', num2str(src.UserData.pktCount) ])

    dataReadyToProc = dataHandeler.checkBuffer(src, dataHand);

    if(dataReadyToProc)
        blockTime_ms = src.UserData.dataBlock(src.UserData.pktsPerBlock, 1) - src.UserData.dataBlock(1, 1);
        expectedTime_ms = 1000*(src.UserData.pktsPerBlock-1)/src.UserData.daqFreq;

        timeError_ms = blockTime_ms - expectedTime_ms;
        timeError_pt = 100*timeError_ms/expectedTime_ms;
        if(abs(timeError_pt) < 1)
            %disp(['Data ready to process: ', num2str(src.UserData.pktCount)]);

            % Process Data
            %tic;
            features = dataProc.featExt(src.UserData.dataBlock(1:src.UserData.pktsPerBlock, :), src.UserData.daqFreq); %pass the first pkesPerBlick colums. Keep the timestamps
            %disp(['Processing data on: ', num2str(src.UserData.nCh), 'ch '])
            %toc
        
            % Trigger
            %tic;
            testHID = false;
            if(~testHID)
                tic
                [gesture, scores] = src.UserData.mlModel.predictFcn(features);
                toc
            else
                % *** For testing *** %
                src.UserData.aCount = src.UserData.aCount + 1;
                if src.UserData.aCount < 100
                    src.UserData.aCount = src.UserData.aCount +1;
                else
                    src.UserData.thisCmd = src.UserData.thisCmd +1;
                    src.UserData.aCount = 0;
                end
    
                if src.UserData.thisCmd > 8 
                    src.UserData.thisCmd = 0;
                end
                gesture = src.UserData.thisCmd;
                % *** End testing *** %
            end
        
            % Map
            % 1 = 1, stop
            if(gesture == 3) %LEFT
                gesture = 3;
            end
            if(gesture == 6) % Right
                gesture = 2;
            end
            
            if(gesture == 31) %Up
                gesture = 5;
            end
            if(gesture == 32) % Down
                gesture = 4;
            end
           
            % Send Command to HID
            %disp("This is a: " + num2str(gesture) + ", scores:" + num2str(scores))
            % One byte to command, just the gesture
            hidDev.write(0xAA, "uint8"); % The SYNCBYTE
            hidDev.write(gesture, "int8");
    
            % Two bytes to program. -1*gesture, Char to map
            %hidDev.write(0xAA, "uint8"); % The SYNCBYTE
            %hidDev.write(-gesture, "int8");
            %hidDev.write(0x58, "uint8"); % 58 = 'X'
        else
            hidDev.write(0xAA, "uint8"); % The SYNCBYTE
            hidDev.write(0, "int8"); % Send stop
            disp(['expected: ', num2str(expectedTime_ms), ', got: ', num2str(blockTime_ms), ' Error: %', num2str(timeError_pt)]);
        end
        % Un comment to find how much headrooom we have:
        % We don't keep up past 18ms prior to adding featExt, predictFcn, and send data
        %java.lang.Thread.sleep(24); 
        %toc;
        
    end
    %toc;
   
end


%% ReadMe
% -----   MatLab frontEnd  (Real Time Runing) --------
% *****   Communications   *****
% Set your serial port for the HID : tty.usbmodem003806534
% Set port for DAQ                 : tty.usbmodem113401
% Start/Stop with the Gui
% *****    Modelm     *** 
% Make sure the correct model file/model name is in

% -----   MatLab recordDataBlocks (Record data for offline analysis)  --------

% -----   MatLab trainer (Anilize, cut, train)  --------


% -----   MatLab Daq  (Record data) --------
% Safe boot: If you hold pin 12 to GND during boot, the system
%   will not init the serial, and will blink the heartbeat fast
% Note, when running freeboard 300Hz is the fastest the Uno will send the
% data at. The matlabe code has no problem keeping up
% However the Arduino clock is not giving me better than 10 to 11 ms
% presison

% Serial send
% The serial sendMY is overloaded, so add the number of channles you want
%   up to 8 (we can make more if we want)
% Place the line nside the a safeboot check ex for 5 data ch:
%   Under the data freq you want

% Ex:
% if(taskClockCycles25Hz == 4) // 25Hz
% {
%   if(!safeBoot){ 
%   daqSerial.sendMY(systemTaskClock, pi, 2*pi, 3*pi, 4*pi, 5*pi);}
%   }
% }


% -----   MatLab HID (Rx gesture from matlab, control computer)   --------
% NOTE: !!!Never, ever have the serial pugged in with the board unpoered!!!
%
% Seems to want a ping from serial before it boots.
%
% *** LED Status  ***
% Blue Slow : No data for 100ms
% Green Slow: Got data
% BlueGreen : Program Mode
% Red fast: Safe mode bootup


%% ToDo
%Matlab side
% Feature extraction
% desistion making
% send to HID

%Arduino Side 
% Set up soft serial for debuging, the R4 won't work with altsoftserial
