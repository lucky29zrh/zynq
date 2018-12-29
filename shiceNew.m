%添加功能:   去除底噪，即根据自适应噪声估计算法估计噪声，去除大部分噪声，对TRF时频滤波
            跳频信号的包络，进而为后面估计参数做准备
%未添加功能：去除定频和扫频噪声
%           对实测的时频矩阵进行max取得最大值序列并不是理想的脉络
%由于目前实验数据中跳频信号未周期性出现，且跳频信号不是
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

%% 打印时频矩阵TFR=SIFT
% %利用了SIFT3
% surf(F, T,  STFT')
% shading interp
% axis tight
% box on
% view(0, 90)
% set(gca, 'FontName', 'Times New Roman', 'FontSize', 14)
% xlabel('Frequency, Hz')
% ylabel('Time, s')
% title('Amplitude spectrogram of the signal')
% handl = colorbar;
% set(handl, 'FontName', 'Times New Roman', 'FontSize', 14)
% ylabel(handl, 'Magnitude, dB')
% 
% figure(); 
% mesh(abs(STFT));   %画三维图  
% title('STFT-时频分布图 汉明窗（127）'); 
% xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
%  
% figure(); 
% N = 2^nextpow2(length(signal)); 
% contour(T,F,abs(STFT));  %画等高线 
% title('STFT contour 汉明窗（127）'); 
% xlabel('Time'),ylabel('Normalized frequency'); 
% 
% 
% %跳频图
% figure();
% surf(T,F,STFT)
% shading interp
% axis tight
% box on
% view(0, 90)
% set(gca, 'FontName', 'Times New Roman', 'FontSize', 14)
% ylabel('Frequency, Hz')
% xlabel('Time, s')
% title('Amplitude spectrogram of the signal')
% handl = colorbar;
% set(handl, 'FontName', 'Times New Roman', 'FontSize', 14)
% ylabel(handl, 'Magnitude, dB')

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
%根据权值数组找出拐点
alpha = 0:0.2:15;
c_alpha= zeros(1,length(alpha));
th_alpha= zeros(1,length(alpha));
for i=1:length(alpha)
    th_alpha(i) = alpha(i)*sumtrf/nt/nf;
    for n=1:nt
        for m=1:nf
           if TFR(m,n)>=th_alpha(i)
               c_alpha(i) = c_alpha(i)+1;
           else
               c_alpha(i) = c_alpha(i)+0;
           end
        end
    end
end
figure()
% find (c_alpha(max(c_alpha)*0.2))
plot(alpha,c_alpha,'*');

% figure()
% subplot(3,1,1)
% plot(c_alpha);
% subplot(3,1,2)
% plot(diff(c_alpha));
% subplot(3,1,3)
% plot(diff(diff(c_alpha)));
%根据拐点进行噪声滤除
% th = 5*sumtrf/nt/nf;
guaidian_index = round(length(c_alpha)*0.35);
guaidian_th = alpha(guaidian_index); %第25%大对应的索引
text(alpha(guaidian_index),c_alpha(guaidian_index),'o','color','r');
th = guaidian_th*sumtrf/nt/nf;
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

%% 打印去除噪声后的的时频矩阵TFR1
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

%% 根据论文进一步去除干扰信号
% 扫频信号 噪声 定频信号
TFR4 = TFR1;

%% 获取最大值序列
[S,F]=max(TFR4); %a row vector containing the maximum element from each column. S是行向量,对应每一列的max value，F是对应max index
figure();
plot(S);
title('跳频信号峰值包络y(n)');
xlabel('时间/us');
ylabel('峰值幅值');
