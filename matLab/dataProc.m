classdef dataProc
    properties (Access = public)
        % nFeat;
        % 
        % fFeat;
        % dataBlock;
    end % properties


    methods (Static, Access = public)%with args  
        function ret = init()
            %ret.nFeat = 6;
            %ret.dataBlock = nan(nCh, nFeat);
        end
        
        function features = featExtTranF(dataBlock)
            nCh = size(dataBlock,2) -1;
            useHann = true;
            removeDC = true;
            magOnly = false;
            appendFreqs = true; % we want the freq info
            
            refCh = 6;
            features = []; % note, if this works prepopulate
            [fftDataReIm] = calcFFT(dataBlock, useHann,removeDC, magOnly, appendFreqs); %FFT needs the data with the timestamps

            refChFftData = fftDataReIm(:,refCh);
            fftDataTF = fftDataReIm(:,2:nCh+1)./refChFftData;
            fftDataTF = abs(fftDataTF);
            for thisCh= 1:nCh
                if(thisCh ~= refCh)
                     features = horzcat(features, fftDataTF(:,thisCh)');
                end
             end
            % % Or just a single ch
            % features = horzcat(features, fftDataTF(:,1)');


        end

        function features = featExt(dataBlock, daqFreq)
            nCh = size(dataBlock,2) -1;
            useHann = true;
            removeDC = true;
            magOnly = true;
            appendFreqs = true; % we want the freq info
            
            % a couple of band gap filters for the 60 and 120
            % for thisCh = 2: nCh +1
            %     dataBlock(:,thisCh) = bandstop(dataBlock(:,thisCh),[57 63],daqFreq);
            %     dataBlock(:,thisCh) = bandstop(dataBlock(:,thisCh),[117 123],daqFreq);
            % end

            % Subtrack the mean
            blockMean = mean(dataBlock(:, 2:end)); %Mean x, y, z
            data = dataBlock(:, 2:end) - mean(blockMean);

            % Time Domain Trivial Features
            blockSTD = std(data);
            blockPeak = peak2peak(data); % p2p x, y, z
            blockRMS = rms(data);
            blockMABS = mad(data); % ends up being harmfull

            % Time Domain for loop Features

            % Frequency Domain Features
            % Trivial Freq Domain Features
            %tic
            [fftData] = calcFFT(dataBlock, useHann,removeDC, magOnly, appendFreqs, 0); %FFT needs the data with the timestamps
            %toc
            zeroHz = fftData(1, 2:end);
            fftDataMag = fftData(); % already in Mag

            % Freq domain loop features
            %Avoid the 60Hz like the plauge that it is.
            fMin = 60; % put in funArgument
            fMax = 100;
            
            %for FMEAN
            sumIxF = zeros(1,nCh);
            sumI = zeros(1,nCh);
            for i = 1:size(fftDataMag,1)
                if fftData(i,1) > fMin  % Start at more than the fMin, avoid the 60Hz at all costs
                    %disp(['i: ', num2str(i), ', size fftDataMag: ', num2str(size(fftDataMag,1)), ', size sumI:', num2str(size(sumI))])

                    sumI = sumI + fftDataMag(i,2:end);
                    sumIxF = sumIxF + fftData(i,1)* fftDataMag(i,2:end);
                elseif fftData(i,1) > fMax
                    break;
                end  
            end
            fMeanBlock = sumIxF./sumI;
            
            % We have pulse train data. Peaks are useless
            % nPeaks = 2; % How many fft peaks to count
            % fftPks = zeros(nCh, nPeaks); % How big?
            % fftPksLocs = zeros(nCh, nPeaks); % How big?
            % for i = 2:nCh+1 % was fftBlockSize = size(fftDataMag,2);
            %     [pks, pksLoc] = findpeaks(fftDataMag(:,(i)), fftDataMag(:,1), 'SortStr','descend', 'MinPeakDistance', 0.5,'NPeaks', nPeaks);
            %     pks = resize(pks, [1,nPeaks]); % We have MAX of NPeaks, but need to be consistant
            %     pksLoc = resize(pksLoc, [nPeaks,1]); % We have MAX of NPeaks, but need to be consistant, loc is flipped
            % 
            %     fftPks(i-1,:) =  pks;
            %     fftPksLocs(i-1,:) =  pksLoc'; % Loc is fliped
            % end

            % Gather up 
            features = horzcat(blockMean, blockSTD, blockPeak, blockRMS, blockMABS, ... 
                               zeroHz, fMeanBlock);
                               % blockMABS
                               % reshape(fftPksLocs, 1,[]), reshape(fftPks, 1,[])); 
                            
        end

    end % methods
end