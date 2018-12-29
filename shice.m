close all;
clear all;

Fs = 61.44e6;        % Sampling frequency
% T = 1/Fs;   
data =importdata('162915.csv');
dataI = data(:,1);
dataQ = data(:,2);
S = dataI + j*dataQ;

%频谱图
f = -Fs/2 :Fs/length(S): (Fs/2 - Fs/length(S));  
figure(); 
plot(f,abs(fftshift(fft(S)))) %接收信号的频域图 
title('接收信号的频域图 ');

%传参
signal = S;
nsc    = 100;                     %每个窗口的长度,也即海明窗的长度
nff    = max(256,2^nextpow2(nsc));%每个窗口进行fft的长度
nov    = 0;                       %重叠率
fs     = Fs;                      %采样率

figure()
% subplot(2,1,1);
spectrogram(signal,hamming(nsc, 'periodic'),nov,nff,Fs);
title('spectrogram函数画图')
[spec_X,spec_f,spec_t]=spectrogram(signal,hamming(nsc, 'periodic'),nov,nff,Fs);
figure()
% 自己实现的stft
[STFT,F,T] = stft(signal,nsc,nov,fs);
%利用了SIFT3
surf(F, T,  STFT')
shading interp
axis tight
box on
view(0, 90)
set(gca, 'FontName', 'Times New Roman', 'FontSize', 14)
xlabel('Frequency, Hz')
ylabel('Time, s')
title('Amplitude spectrogram of the signal')
handl = colorbar;
set(handl, 'FontName', 'Times New Roman', 'FontSize', 14)
ylabel(handl, 'Magnitude, dB')

figure(); 
mesh(abs(STFT));   %画三维图  
title('STFT-时频分布图 汉明窗（127）'); 
xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
 
figure(); 
N = 2^nextpow2(length(signal)); 
contour(T,F,abs(STFT));  %画等高线 
title('STFT contour 汉明窗（127）'); 
xlabel('Time'),ylabel('Normalized frequency'); 


%跳频图
figure();
surf(T,F,STFT)
shading interp
axis tight
box on
view(0, 90)
set(gca, 'FontName', 'Times New Roman', 'FontSize', 14)
ylabel('Frequency, Hz')
xlabel('Time, s')
title('Amplitude spectrogram of the signal')
handl = colorbar;
set(handl, 'FontName', 'Times New Roman', 'FontSize', 14)
ylabel(handl, 'Magnitude, dB')

%对自己实现的stft得到的STFT进行时频滤波
TFR = STFT;
%去噪后的二维时频分布TFR2
nf = size(TFR, 1);
nt = size(TFR, 2);
sumtrf=0;
for n=1:nt
    for m=1:nf
        sumtrf = sumtrf+TFR(m,n);
    end
end
th = 0.2*sumtrf/nt/nf;
TFR1 = zeros(m,n);
for n=1:nt
    for m=1:nf
       if TFR(m,n)>=th
           TFR1(m,n) = TFR(m,n);
       else
            TFR1(m,n) = 0;
       end
    end
end
%跳频图
figure();
surf(T,F,TFR1)
shading interp
axis tight
box on
view(0, 90)
set(gca, 'FontName', 'Times New Roman', 'FontSize', 14)
ylabel('Frequency, Hz')
xlabel('Time, s')
title('Amplitude spectrogram of the signal')
handl = colorbar;
set(handl, 'FontName', 'Times New Roman', 'FontSize', 14)
ylabel(handl, 'Magnitude, dB')

figure(); 
mesh(abs(TFR1));   %画三维图  
title('STFT-时频分布图 汉明窗（127）'); 
xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
 
figure(); 
contour(T,F,abs(TFR1));  %画等高线 
title('STFT contour 汉明窗（127）'); 
xlabel('Time'),ylabel('Normalized frequency'); 
