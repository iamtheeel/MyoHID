%
% Joshua Mehlman and James Swartz
%
% ENGR 845 Spring 2024
% Term Project:
%

%
% Cut up the files in to blocksize, windowInc that will be used
% Send to feature extractor
%

% data dir: What directory the data lives in
% fileList: An array of files names without the _trimmed.mat
% dataLabels: An array of the labels of each file
% blockSize: Desired processing block size
% windowInc: The amount of data to increment


function [features] = blockExtract(dataDir, fileList, dataLables, blockSize, windowInc, testTrain, daqF) %wl, wInc)
    nFiles = size(fileList, 2);
    features = [];

    expectedTime_ms = 1000*(blockSize-1)/daqF;

    for fileNum = 1:nFiles
        thisFileName = fileList(fileNum);
        thisFile = strcat(dataDir, "/", testTrain, '/', thisFileName, ".mat")
        load(thisFile, "block");

        thisLabel = dataLables(fileNum);

        dataLen = size(block, 1); % How many time points in our dataset
        
        startPt = 1;
        endPt = blockSize;

        while 1
            if endPt > dataLen; break; end

            % Cut off a block
            thisDataBlock = block(startPt:endPt,:);

            % Do we have a time gap?
            blockTime_ms = thisDataBlock(end,1) - thisDataBlock(1,1);
            timeError_ms = blockTime_ms - expectedTime_ms;
            timeError_pt = 100*timeError_ms/expectedTime_ms;
            if(abs(timeError_pt) < 1)
                %tic;
                blockFeatures = dataProc.featExt(thisDataBlock, daqF); 
                %blockFeatures = dataProc.featExtTranF(thisDataBlock); % This was comicly bad.
                %toc;
    
                % Add the label
                blockFeatures = horzcat(blockFeatures, thisLabel); % All the features in a file have the same lable
                
                % Add to our total features
                features = vertcat(features, blockFeatures);
                %toc
            else
                disp(['expected: ', num2str(expectedTime_ms), ', got: ', num2str(blockTime_ms), ' Error: %', num2str(timeError_pt)]);
            end

            startPt = startPt + windowInc;
            endPt = endPt + windowInc;
    

       end % Each Block
    end % Each file
    
    disp(['Done processing Features'])
end % End function

