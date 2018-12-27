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
