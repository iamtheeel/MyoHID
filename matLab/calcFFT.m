%
% Joshua Mehlman and James Swartz
%
% ENGR 845 Spring 2024
% Term Project:
%

% Do and plot an fft of the data
function [fftData] = calcFFT(data, useHann, removeDC, magOnly, appendFreqs, Fs)
   
    %%Get the variables
    if(nargin < 5)
        Fs = 0;
    end
    if(Fs == 0)
        t0 = data(1,1);
        t1 = data(end,1);
        deltaT = ((t1 - t0)/(size(data,1)-1))/1000;
        Fs = 1/deltaT; % Sample frequency (Hz)
        Fs = round(Fs);
    end
    %deltaT = data(2,1)/1000 - data(1,1)/1000 % Data in ms
    %Fs = 1/deltaT % Sample frequency (Hz)

    nPts = size(data,1);
    L =  floor((nPts+1)/2);
    data = data(:, 2:end); % Strip the time
    %% mean(mean(data))
    if(removeDC)
        data = data - mean(mean(data));
    end
    
    %% Window
    if(useHann)
    fftWin = hann(nPts);
    windowedInput = data.*fftWin;
    else
        windowedInput = data;
    end
    
    %% FFT
    
    fftData = fft(windowedInput);
    fftData = fftData(1:L, :); % Nyquest
    if(magOnly)
        fftData = abs(fftData);
    end

    if(appendFreqs)
        freqs = (Fs/2)/L*(0:L-1);
        freqs = freqs';
        
        fftData = horzcat(freqs, fftData);
    end
end