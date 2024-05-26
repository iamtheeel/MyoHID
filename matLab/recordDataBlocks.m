%
% Record data for training
% Joshua Mehlman and James Swartz
%
% ENGR 845 Spring 2024
% Term Project:
%


% User vars
%serialPort = "/dev/tty.usbmodem113401"; % UNIX format
% 115200 (default) | 50 | 75 | 110 | 134 | 150 | 200 | 300 | 600 | 1200 | 1800 | 2400 | 4800 | 9600 | 19200 | 38400 | 57600 | 230400
serialSpeed = 2000000;
serialPort = "COM9" % Windows Format
nCh = 4; % how many ch of data

nGestures = 32;
nPacketsInBlock = 1024;
nPacketsForOverlap = nPacketsInBlock-1;% As close to 0% as works 



%Data save 
datetime.setDefaultFormats('default','yyyy-MM-dd-hhmmss');
dataAcqStartTime = datetime('now');
mkdir(strcat('../Data/', string(dataAcqStartTime), '/test'))
mkdir(strcat('../Data/', string(dataAcqStartTime), '/train'))


%start up the serial
clear serialDev;
serialDev = serialport(serialPort, serialSpeed);
configureTerminator(serialDev, "CR/LF");
serialDev.Terminator; % Set serial to use terminator

serialDev.UserData = struct("dataBlock",[],...
                            "nBytesPerPacket", 0, ...
                            "pktCount",0, ...
                            "pktsPerBlock", nPacketsInBlock, ...
                            "nReads", 0,...
                            "nCh", nCh,...
                            "testTrain", 'train', ...
                            "gesture", nGestures, ...
                            "saveNums", zeros(1,nGestures), ...
                            "testSaveNums", zeros(1,nGestures), ...
                            "trainSaveNums", zeros(1,nGestures), ...
                            "dataAcqStartTime", dataAcqStartTime, ...
                            "nGest", nGestures);


% serialDev.flush(); % Make sure we start from 0

% Configs and one times
clear dataHand;
dataHand = dataHandeler.init(nPacketsInBlock, nPacketsForOverlap);

pushbuttonPlot(serialDev, dataHand) % Run the shooting match

function pushbuttonPlot(sDev, dataH)
    figPos_x = 100;
    figPos_y = 300;
    figHeight = 600;
    figWidth = 600;
    f = figure;
    f.Position = [figPos_x figPos_y, figWidth, figHeight]; 
    % Start/Stop control
  
    % serial port list
    % Train Model btn
    
    % Test train dropdown 
    fromTop = 10;
    testTrain_text = uicontrol(f, "Style","text", 'Position',[0, (1), 100, figHeight - fromTop]);
    testTrain_text.String = "Test/Train";
    testTrain_dd = uicontrol(f, "Style", 'popupmenu', 'Position',[75, (1), 100, figHeight - fromTop]); % posX, ??, width, posY
    testTrain_dd.String = {'train', 'test'};
    testTrain_dd.Callback = @(src, evt) testTrain(src, evt, sDev);
    
    % gesture number dropdown
    fromTop = 35;
    gestNum_text = uicontrol(f, "Style","text", 'Position',[0, (1), 100, figHeight - fromTop]);
    gestNum_text.String = "Gesture";
    gestNum_dd = uicontrol(f, "Style", 'popupmenu', 'Position',[75, (1), 75, figHeight - fromTop]); % posX, ??, width, posY
    gestList = 1:sDev.UserData.gesture;
    gestNum_dd.String = compose("%d", gestList);
    gestNum_dd.Callback = @(src, evt) gestNumSel(src, evt, sDev);

    % gesture capture sumary
    fromTop = 10;
    gestSave_text = uicontrol(f, "Style","text", 'Position',[200, (1), 100, figHeight - fromTop]);
    gestSave_text.String = "Save Count";
    vars = ["train","test", "total"];
    tableWidth = 260;
    gestSave_tbl = uitable(f, "Data", zeros(sDev.UserData.gesture, 3), "ColumnName", vars, Position=[300,1,tableWidth, figHeight-fromTop]); %x, y, width, height
    sDev.UserData.gesture = 1;  % we are done using this as the count, so we can set the default to "1"
    fromTop = 75;
    readMe_txt = uicontrol(f, "Style","text", 'Position',[1, (1), 300, figHeight - fromTop]);

    readMe_txt.String = compose("Saves Dir: ../../data/YYYY-MM-DD-HHMMSS/test|train/\n\n FileName:timeDate_pXX_YY.mat");

    startDAQ = uicontrol(Position=[20 45 60 20]);
    startDAQ.String = 'Start DAQ';
    startDAQ.Callback = @(src, evt) startDAQBntPress(src, evt, sDev, dataH);

    stopDAQ = uicontrol(Position=[20 20 60 20]);  % Position [left, bottom, width, height]
    stopDAQ.String = 'Stop DAQ';
    stopDAQ.Callback = @(src, evt) stopDAQBntPress(src, evt, sDev, gestSave_tbl);  


    function startDAQBntPress(src,evt, sDev, dataHa)
        disp("Starting DAQ!");
        disp(sDev.UserData.testTrain);
        sDev.flush(); % Make sure we start from 0
        % Register the serial callback to start data
        configureCallback(sDev,"terminator", @(src, evt) readSerialData(src, evt, dataHa)) 
    end

    function stopDAQBntPress(src,evt, sDev, gestSave_tbl)
        configureCallback(sDev, "off");
        for i = 1:size(sDev.UserData.saveNums,2)
            gestSave_tbl.Data(i,1) = sDev.UserData.trainSaveNums(i);
            gestSave_tbl.Data(i,2) = sDev.UserData.testSaveNums(i);
            gestSave_tbl.Data(i,3) = sDev.UserData.saveNums(i);
           
        end
        sDev.UserData.dataBlock = [];
        sDev.UserData.pktCount =0;
        sDev.UserData.nReads = 0;
        disp("DAQ Stopped!");
    end

    function testTrain(src,evt, sDev)
        str = testTrain_dd.String; % The list of values
        sDev.UserData.testTrain = str{testTrain_dd.Value}; % Selected item from the list
        disp(sDev.UserData.testTrain);
    end

    function gestNumSel(src,evt, sDev)
        sDev.UserData.gesture = gestNum_dd.Value; % Selected item from the list
        disp("Gesture: " + num2str(sDev.UserData.gesture));
    end


    %
    % Add a list of models to load
    %

    




end


function readSerialData(src, evt, dataHand)% src is the serialPort with all the variables in it
    tic;
    dataHand = dataHandeler.dataPacket_Min(src, dataHand);
    %toc
    % Configs and disposable data in dataHand
    %disp(['data: ', dataHand.packet ])

    % Persistant Data must be in src
    %disp(['Count: ', num2str(src.UserData.pktCount) ])

    dataReadyToProc = dataHandeler.checkBuffer(src, dataHand);
    if(dataReadyToProc)
        % grab the block and append
        block = src.UserData.dataBlock(1:src.UserData.pktsPerBlock, :);

      
        % save dir '/2024-04-06-052622/test/' or '2024-04-06-052622/train/'
        % fileName  YYY-MM-DD-HHMMSS_pxx_yy.mat
        % xx is gesture number, yy is save number
        gesture = src.UserData.gesture;
        src.UserData.saveNums(gesture) = src.UserData.saveNums(gesture) + 1; % Increment the save number for this gesture
        if(strcmp(src.UserData.testTrain,'train')) % keep track of how many for test, and train
            src.UserData.trainSaveNums(gesture) = src.UserData.trainSaveNums(gesture) + 1;
        else
            src.UserData.testSaveNums(gesture) = src.UserData.testSaveNums(gesture) + 1;
        end
         
        dateTime = string(src.UserData.dataAcqStartTime);
        
        saveDir = strcat('../Data/', dateTime, '/', src.UserData.testTrain);
        fileName = strcat(dateTime, '_p', num2str(gesture), '_', num2str(src.UserData.saveNums(gesture)), '.mat');

        disp("Saving block: " + num2str(size(block)) + ", as: " + fileName + ", in: " + saveDir)
        save(strcat(saveDir, '/', fileName), 'block'); 
    end
end


%% ReadMe
% MatLab
% Set your serial port
% Start/Stop with the Gui

% Arduino
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

%% ToDo
%Matlab side
% Feature extraction
% desistion making
% send to HID

%Arduino Side 
% Set up soft serial for debuging, the R4 won't work with altsoftserial
