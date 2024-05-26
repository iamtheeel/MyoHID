classdef dataHandeler
    properties (Access = public)
        % Data Structure
        syncBytes;

        % Data block structure
        nChInPkt;
        nPacketsInBlock;
        nPacketsOverlap;
        bufferSize;

        % Are we keeping up?
        nBytesOverHead;
        nBytesPerCh;
    end

    methods (Static, Access = public)%with args  
        function app = init(nPacketsInBlock, nPacketsOverlap)
            % From main
            app.nPacketsOverlap = nPacketsOverlap;
            app.nPacketsInBlock = nPacketsInBlock;
            app.bufferSize = nPacketsInBlock + nPacketsOverlap;

            %app.nBytesOverHead = 14;  %our structure has 12 bytes of overhead
            app.nBytesOverHead = 7;  %our structure has 12 bytes of overhead, including the timeStamp

            app.nBytesPerCh = 2;      %Our data is signed int (int16)
            app.nBytesPerPacket = 0;
        
            % Packet info
            %app.nChInPkt = nCh;  % add to function call
            app.syncBytes = [0x33, 0xFF];
            app.syncByte = 0xAA;
        end

        function dataReady = checkBuffer(src, app)
            dataReady = false;

            % disp(['packetCount: ', num2str(packetCount), ...
            %       ', app.nPacketsInBlock: ', num2str(app.nPacketsInBlock), ...
            %       ', app.nPacketsForOverlap: ', num2str(app.nPacketsOverlap), ...
            %       ', nReads: ', num2str(nReads)])

            if ( ((src.UserData.nReads == 0)  && (src.UserData.pktCount >= app.nPacketsInBlock)) || ...
                 ((src.UserData.nReads > 0)  && (src.UserData.pktCount >= app.nPacketsInBlock + app.nPacketsOverlap)) )

                if(src.UserData.nReads > 0) 
                    % Rotate the buffer (don't rotate our first block worth of data)
                    %disp(["Rotate data: ", num2str(app.nPacketsOverlap)])
                    src.UserData.dataBlock = circshift(src.UserData.dataBlock, -app.nPacketsOverlap, 1); % shift up by overlap time = 0.272 milisec
                end
                src.UserData.pktCount = app.nPacketsInBlock; % after processing the next packet goes at the end of the block (after circshift)

                src.UserData.nReads = src.UserData.nReads +1;
                dataReady = true;
            end
            return;
        end

        function app = dataPacket_Min(src, app)
            % Are we keepting up
            packetsAvailable = (src.NumBytesAvailable()/src.UserData.nBytesPerPacket);
            if(src.UserData.nBytesPerPacket > 0) && (app.nPacketsOverlap > 0) % Don't check on the first run
                if(packetsAvailable > app.nPacketsOverlap) % Don't panic 
                    disp(['dataHendeler.m:dataPacket_Min: !!!We are not keeping up!!!!  packetsAvailable: ', num2str(packetsAvailable)]);
                    disp(['nBytesAvail: ', num2str(src.NumBytesAvailable()), ', app.nBytesPerPacket: ', num2str(src.UserData.nBytesPerPacket), ', app.nPacketsOverlap: ', num2str(app.nPacketsOverlap)]);
                    
                    src.flush();
                    return;
                % elseif (src.NumBytesAvailable() > src.UserData.nBytesPerPacket)
                %     disp(["Running tight, but doing ok. Packes avail: ", num2str(packetsAvailable)])
                end
            end

            %pktSyncByte = read(src, 1, "uint8");
            if(read(src, 1, "uint8") ~= app.syncByte)
                disp("*** bad packet SYNC Byte 1 Error should be. 51:255")
                src.flush();
                return;
            end
            pktTime = read(src, 1, "uint32");           % Get the timestamp
            packet = read(src, src.UserData.nCh, "int16");     % Get the data
            packet = cat(2, pktTime, packet);       % Add the timestamp to the datablock
            
            read(src, 2, "uint8"); % read the terminating bytes


            % Persistant Data must be in src
            %if size(src.UserData.dataBlock, 1)   == 0 % Our first run
            if src.UserData.nBytesPerPacket == 0 % Our first run
                % if this is our first block init the data
                dataSize = size(packet, 2);  %How many datapoints (including time)
                %src.UserData.nCh = app.nChInPkt;

                disp(['creating data buffer ',num2str(app.bufferSize), ' x ', num2str(dataSize)])
                src.UserData.dataBlock = nan(dataSize, app.bufferSize)';

                disp(['nBytesOverHead: ', num2str(app.nBytesOverHead), ', app.nBytesPerCh: ', num2str(app.nBytesPerCh), ', nCh: ', num2str(src.UserData.nCh)]);
                src.UserData.nBytesPerPacket = app.nBytesOverHead + app.nBytesPerCh*src.UserData.nCh; % we want this to see if we are keeping up 
                disp(['nBytesPerPacket: ', num2str(src.UserData.nBytesPerPacket)]);
            end

            src.UserData.pktCount = src.UserData.pktCount +1;           % Increment the packet count
            src.UserData.dataBlock(src.UserData.pktCount,:) = packet;   % Slot the new datapoint where it belongs

        end


    end % Methods

end % Class
