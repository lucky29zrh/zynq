功能：针对实数是stft

----
stft.m
---

function [S,F,T] = stft(signal,nsc,nov,fs)
%% 使用fft实现周期图法
% 封装为函数:信号S输入为列向量，如果为行向量则转置。
% 考虑信号为复数，不需要折半处理
% function [ S , W , T ] = mf_spectrogram...
%     ( signal , nsc , nov , fs )
% Input        
%       signal      - Signal vector         输入信号(行向量)
%       nsc         - Number SeCtion        每个小分段的信号长度
%       nov         - Number OverLap        重叠点数
%       fs          - Frequency of Sample   采样率
% Output
%       S           - A matrix that each colum is a FFT for time of nsc 
%                    如果nfft为偶数，则S的行数为(nfft/2+1)，如果nfft为奇数，则行数为(nfft+1)/2   
%                    每一列是一个小窗口的FFT结果，因为matlab的FFT结果是对称的，只需要一半
%       F           - A vector labeling frequency   频率轴
%       T           - A vector labeling time        时间轴
% Signal Preprocessing

if size(signal,2) > 1
	signal =signal';
end
h = hamming(nsc, 'periodic');        % Hamming weight function  海明窗加权，窗口大小为nsc
L = length(signal);                  % Length of Signal         信号长度
nstep  = nsc-nov;                    % Number of STep per colum 每个窗口的步进
ncol   = fix( (L-nsc)/nstep ) + 1;   % Number of CoLum          信号被分成了多少个片段
nfft   = max(256,2^nextpow2(nsc));   % Number of Fast Fourier Transformation  每个窗口FFT长度
nrow   = nfft/2+1;

%
%
STFT1 = zeros(nfft,ncol);  %STFT1 is a matrix 初始化最终结果
index  = 1;%当前片段第一个信号位置在原始信号中的索引
for i=1:ncol
    %提取当前片段信号值,并用海明窗进行加权(均为长度为nsc的行向量)
    temp_S=signal(index:index+nsc-1).*h;
    %对长度为nsc的片段进行nfft点FFT变换
    temp_X=fft(temp_S,nfft);
    %将长度为nfft点(行向量)的fft变换转换为列向量，存储到矩阵的列向量
    STFT1(:,i)=temp_X;
    %将索引后移
    index=index + nstep;
end

% -----当输入信号为实数时，对其的操作（也就是说只考虑信号的前一半）
% 由于实数FFT的对称性,提取STFT1的nrow行
STFT_X = STFT1(1:nrow,:);             % Symmetric results of FFT

% % Axis Generating
% fax =(0:(nrow-1)) * fs./nfft;                        % Frequency axis setting
% tax = (nsc/2 : nstep : nstep*(ncol-1)+nsc/2)/fs;     % Time axis generating
%  
%  % Output
%  F = fax;
%  T = tax;
%  S = STFT_X;
 
%如果是为了和标准周期图法进行误差比较，则后续计算(abs,*2,log(+1e-6))不需要做，只需
%plot(abs(spec_X)-abs(STFT1))比较即可

STFT2 = abs(STFT_X/nfft);
%nfft是偶数，说明首尾是奈奎斯特点，只需对2:end-1的数据乘以2
STFT2(2:end-1,:) = 2*STFT2(2:end-1,:);  % Add the value of the other half
%STFT3 = 20*log10(STFT1);                % Turn sound pressure into dB level
STFT3 = 20*log10(STFT2 + 1e-6);          % convert amplitude spectrum to dB (min = -120 dB)

% Axis Generating
fax =(0:(nrow-1)) * fs./nfft;                        % Frequency axis setting
tax = (nsc/2 : nstep : nstep*(ncol-1)+nsc/2)/fs;     % Time axis generating
 
 % Output
 F = fax;
 T = tax;
 S = STFT3;
 

end



-----
test.m
-----
%%------------------------------------------------------------------------
%%功能：使用自定义stft对特定信号做短时傅里叶分析(matlab api和自己实现的stft)
%       测试自己实现的逆短时傅里叶变换istft
% 时频工具箱tfrstft tfristft
%%------------------------------------------------------------------------
close all;
clear all;
Fs=1024;
t=0:1/Fs:2;
%输入实信号
x=cos(2*pi*50*t).*(t>=0&t<1)+0.5*cos(2*pi*150*t).*(t>=1&t<2);
figure();
f= -Fs/2:Fs/length(x):Fs/2-Fs/length(x);
plot(f,abs(fftshift(fft(x))));

% Fs = 61.44e6;        % Sampling frequency
% % T = 1/Fs;   
% data =importdata('162915.csv');
% dataI = data(:,1);
% dataQ = data(:,2);
% S = dataI + j*dataQ;
% 
%传参
signal = x;
nsc    = 100;                     %每个窗口的长度,也即海明窗的长度
nff    = max(256,2^nextpow2(nsc));%每个窗口进行fft的长度
nov    = 0;                       %重叠率
fs     = Fs;                      %采样率

[STFT,F,T] = stft(signal,nsc,nov,fs);
%利用了SIFT3
figure()
subplot(2,1,1);
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

subplot(2,1,2);
spectrogram(signal,hamming(nsc, 'periodic'),nov,nff,Fs);
title('spectrogram函数画图')
[spec_X,spec_f,spec_t]=spectrogram(signal,hamming(nsc, 'periodic'),nov,nff,Fs);



