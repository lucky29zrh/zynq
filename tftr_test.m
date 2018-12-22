%输入同一个跳频信号，检测工具箱中不同的时频分析方法的效果：STFT，WVD，PWVD，SPWVD.
clc 
clear all 
close all 
% 利用stft实现了跳频信号的跳周期估计 简单的进行了跳频信号的仿真，然后利用短时傅里叶变换对跳频信号进行分析
%% 产生信号(跳频信号) 
%频率集 
f1 = 7e3;f2 = 2e3; f3= 4e3;  f4= 1e3;  f5= 8e3; f6= 6e3;  f7= 3e3; f8= 5e3;   
 
fs = 20e3; %采样频率 
fh= 200 ;  %跳速 
Th= 1/fh;  %跳周期 

t1 = 0: 1/fs : Th- 1/fs; %第一跳周期内的采样时间点     
t=0:1/fs:8*Th-1/fs; %整段信号的采样时间点 
x1 = cos(2*pi*f1*t1); x2 = cos(2*pi*f2*t1); x3 = -1*cos(2*pi*f3*t1); x4 = -1*cos(2*pi*f4*t1);        
x5 = cos(2*pi*f5*t1); x6 = cos(2*pi*f6*t1); x7 = -1*cos(2*pi*f7*t1); x8 = -1*cos(2*pi*f8*t1); 
s= [x1  x2  x3  x4 x5  x6  x7  x8 ]; %整段接收信号 
 
x=s; 
% x=awgn(x,10,'measured');%加高斯白噪声，信噪比为10dB 
figure(1); 
plot(t,x); %接收信号的时域图 
 
x = x';  
x = hilbert(x);  %       得到解析信号
N = 2^nextpow2(length(x)); 
f = 0 :fs/length(x): (fs - fs/length(x));  
figure(2); 
plot(f,abs(fft(x))) %接收信号的频域图 
%% STFT 
H = window(@hamming,127);      %加窗（汉明窗，@hamming。127个点） 
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
axis([0 N 0 0.5]) 

figure(5); 
imagesc(T,F,abs(TFR1));   %imagesc(A) 将矩阵A中的元素数值按大小转化为不同颜色， 
                          %并在坐标轴对应位置处以这种颜色染色 
title('STFT imagesc 汉明窗（127）'); 

%% WVD
[tfr, t, f] =tfrwv(x,1:length(x),N);
figure(); 
mesh(abs(tfr));   %画三维图  
title('WVD-时频分布图 '); 
xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
 
figure(); 
contour(t,f(1:N/2),abs(tfr(1:N/2,:)));  %画等高线 
title('WVD-时频分布图'); 
xlabel('Time'),ylabel('Normalized frequency'); 
axis([0 N 0 0.5]) 

figure(); 
imagesc(t,f,abs(tfr));   %imagesc(A) 将矩阵A中的元素数值按大小转化为不同颜色， 并在坐标轴对应位置处以这种颜色染色              
title('WVD-时频分布图'); 

%% PWVD
[tfr, t, f] =tfrpwv(x,1:length(x),N);
figure(); 
mesh(abs(tfr));   %画三维图  
title('PWVD-时频分布图 '); 
xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
 
figure(); 
contour(t,f(1:N/2),abs(tfr(1:N/2,:)));  %画等高线 
title('PWVD-时频分布图'); 
xlabel('Time'),ylabel('Normalized frequency'); 
axis([0 N 0 0.5]) 

figure(); 
imagesc(t,f,abs(tfr));   %imagesc(A) 将矩阵A中的元素数值按大小转化为不同颜色， 并在坐标轴对应位置处以这种颜色染色              
title('PWVD-时频分布图'); 
%% SPWVD
[tfr, t, f] =tfrspwv(x,1:length(x),N);
figure(); 
mesh(abs(tfr));   %画三维图  
title('SPWVD-时频分布图 '); 
xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
 
figure(); 
contour(t,f(1:N/2),abs(tfr(1:N/2,:)));  %画等高线 
title('SPWVD-时频分布图'); 
xlabel('Time'),ylabel('Normalized frequency'); 
axis([0 N 0 0.5]) 

figure(); 
imagesc(t,f,abs(tfr));   %imagesc(A) 将矩阵A中的元素数值按大小转化为不同颜色， 并在坐标轴对应位置处以这种颜色染色              
title('SPWVD-时频分布图'); 
