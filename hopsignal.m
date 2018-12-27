%---------跳频信号（加入Alpha稳定分布噪声）的FLOSPWV
clear,clc,close all
%--------产生跳频信号
fs=4*10^3;   %采样频率
fk = [1.1 1.3 1.6 1.0 1.7 1.5 1.2 1.4]*10^3;   %跳频图案集
th=50*10^(-3);   %跳频周期，每跳驻留时间

ze = zeros(1,0);   %0表示信号延迟为零
x = []; sig = ze;
t = 0:1/fs:th-1/fs;
for i = fk;
    x = cos(2*pi*i*t);
    sig = [sig x];   %sig为输出跳频信号
end

N = length(sig);
figure();
plot(sig);
title('调频信号');

figure();
x = sig';  
x = hilbert(x);  %       得到解析信号
N = 2^nextpow2(length(x)); 
f = 0 :fs/length(x): (fs - fs/length(x));  
figure(2); 
plot(f,abs(fft(x))) %接收信号的频域图 
title('接收信号的频域图 ');

%% STFT 
H = window(@hamming,127);      %加窗（汉明窗，@hamming 127个点） 
[TFR1,T,F]=tfrstft(x,1:length(x),N,H);  %短时傅里叶变换函数tfrstft 
                                        %输入为：信号x 
                                        %        信号长度 
                                        %        所加窗函数 
                                        %输出为：短时傅里叶变换得到的时频矩阵 
                                        %        时频矩阵对应的时间序列 
                                        %        时频矩阵对应的频率序列 
                                       
figure(3); 
mesh(abs(TFR1));   %画三维图  
title('STFT-时频分布图 汉明窗（127）'); 
xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
 
figure(4); 
contour(T,F(1:N/2),abs(TFR1(1:N/2,:)));  %画等高线 
title('STFT contour 汉明窗（127）'); 
xlabel('Time'),ylabel('Normalized frequency'); 


figure(5); 
imagesc(T,F,abs(TFR1));   %imagesc(A) 将矩阵A中的元素数值按大小转化为不同颜色， 
                          %并在坐标轴对应位置处以这种颜色染色 
title('STFT imagesc 汉明窗（127）'); 

%% spwvd时频分布
figure();
signal=hilbert(sig);
[tfr,t,f]=tfrspwv(signal');
axis([0 N 0 0.5]) 
imagesc(t,f,abs(tfr));
title('SPWVD时频图');
xlabel('时域采样点');
ylabel('频域采样点');

----
%---------跳频信号（加入Alpha稳定分布噪声）的FLOSPWV
clear,clc,close all
%--------产生跳频信号
fs=8*10^3;   %采样频率4
fk = [1.1 1.3 1.6 1.0 1.7 1.5 1.2 1.4]*10^3;   %跳频图案集
th=50*10^(-3);   %跳频周期，每跳驻留时间

ze = zeros(1,0);   %0表示信号延迟为零
x = []; sig = ze;
t = 0:1/fs:th-1/fs;
for i = fk
    x = cos(2*pi*i*t);
    sig = [sig x];   %sig为输出跳频信号
end
N = length(sig);

%添加噪声和定频信号
sig=awgn(sig,10,'measured');%加高斯白噪声，信噪比为10dB
A1=1;A2=1;  %调制信号幅度
f1=2.5e3;f2=3.5e3;%调制信号频率
t1= [0:N-1]/fs;%采样时间
xt=A1*cos(2*pi*f1*t1)+A2*cos(2*pi*f2*t1);%调制信号
sig=sig+xt;                     %%混合AM信号和跳频信号



figure();
plot(sig);
title('调频信号');

figure();
x = sig';  
x = hilbert(x);  %       得到解析信号
N = 2^nextpow2(length(x)); 
f = 0 :fs/length(x): (fs - fs/length(x));  
figure(2); 
plot(f,abs(fft(x))) %接收信号的频域图 
title('接收信号的频域图 ');

%% STFT 
H = window(@hamming,127);      %加窗（汉明窗，@hamming 127个点） 
[TFR1,T,F]=tfrstft(x,1:length(x),N,H);  %短时傅里叶变换函数tfrstft 
                                        %输入为：信号x 
                                        %        信号长度 
                                        %        所加窗函数 
                                        %输出为：短时傅里叶变换得到的时频矩阵 
                                        %        时频矩阵对应的时间序列 
                                        %        时频矩阵对应的频率序列 
                                       
figure(3); 
mesh(abs(TFR1));   %画三维图  
title('STFT-时频分布图 汉明窗（127）'); 
xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
 
figure(4); 
contour(T,F(1:N/2),abs(TFR1(1:N/2,:)));  %画等高线 
title('STFT contour 汉明窗（127）'); 
xlabel('Time'),ylabel('Normalized frequency'); 


figure(5); 
imagesc(T,F,abs(TFR1));   %imagesc(A) 将矩阵A中的元素数值按大小转化为不同颜色， 
                          %并在坐标轴对应位置处以这种颜色染色 
title('STFT imagesc 汉明窗（127）'); 

%% spwvd时频分布
figure();
signal=hilbert(sig);
[tfr,t,f]=tfrspwv(signal');
axis([0 N 0 0.5]) 
imagesc(t,f,abs(tfr));
title('SPWVD时频图');
xlabel('时域采样点');
ylabel('频域采样点');


%% 功率谱
b = sig';
fs1 = fs;
window=hamming(length(b)); %汉明窗
figure();
% Hs=spectrum.periodogram('Hamming');
% psd(Hs,data,'fs',fs);%%功率谱
% [Pxx,F] = psd(Hs,data,fs,'twosided');
nfft=2048;
[Pxx,f]=periodogram(b,window,nfft,fs1); %周期图法
Pow= (fs1/length(Pxx)) * sum(Pxx);      %平均功率
plot(f,10*log10(Pxx));
title('整段信号的功率谱');
xlabel('Frequency');
ylabel('Power/frequency(dB/Hz)');


%%每一段数据的功率谱
figure();
b1=b(1:512,1);     %分四段
b2=b(513:1024,1);
b3=b(1025:1536,1);
b4=b(1537:2048,1);
% window1=boxcar(length(b1)); %矩形窗
window1=hamming(length(b1)); %汉明窗
subplot(221);
nfft1=2048;
[Pxx1,f1]=periodogram(b1,window1,nfft1,fs1); %周期图法
plot(f1,10*log10(Pxx1));
Pow1= (fs1/length(Pxx1)) * sum(Pxx1);
subplot(222);
[Pxx2,f2]=periodogram(b2,window1,nfft1,fs1); %直接法
plot(f2,10*log10(Pxx2));
subplot(223);
[Pxx3,f3]=periodogram(b3,window1,nfft1,fs1); %直接法
plot(f3,10*log10(Pxx3));
subplot(224);
[Pxx4,f4]=periodogram(b4,window1,nfft1,fs1); %直接法
plot(f4,10*log10(Pxx4));



figure();%整段数据的平均功率谱
Pxx7=(Pxx1.*Pxx2.*Pxx3.*Pxx4);
Pow7= (fs1/length(Pxx7)) * sum(Pxx7);
plot(f4,(10/4)*log10(Pxx7));
title('整段数据的平均功率谱');
xlabel('Frequency');
ylabel('Power/frequency(dB/Hz)');

figure();%对消后的功率谱
% Pxx8=(Pxx1-Pxx7)+(Pxx2-Pxx7)+(Pxx3-Pxx7)+(Pxx4-Pxx7);
 %Pxx8=[(Pxx1/Pxx7);(Pxx2/Pxx7);(Pxx3/Pxx7);(Pxx4/Pxx7)];
Pxx8=10*log10(Pxx)-(10/4)*log10(Pxx7);
%Pxx8=[10*log10(Pxx);0;0;0]-[10*log10(Pxx7);10*log10(Pxx7);10*log10(Pxx7);10*log10(Pxx7)];Pxx8=Pxx8(1:2049,1);
 
Pow8= (fs1/length(Pxx8)) * sum(Pxx8);

plot(f,Pxx8);
title('对消后的功率谱');
xlabel('Frequency');
ylabel('Power/frequency(dB/Hz)');
yita=Pow/Pow8;%功率对消比



figure(); 
imagesc(T,F,Pow8);   %imagesc(A) 将矩阵A中的元素数值按大小转化为不同颜色， 
                          %并在坐标轴对应位置处以这种颜色染色 
title('STFT imagesc 汉明窗（127）'); 
