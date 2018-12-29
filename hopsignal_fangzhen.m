%大致为论文仿真模型。
%功能：对给定的跳频信号进行参数估计
%     2018年12月29日添加了去除噪声序列功能
%缺陷：当添加幅度较强的定频信号时，影响后面时频脊线的绘图，未对TFR做时频滤波处理。

%---------跳频信号（加入Alpha稳定分布噪声）的FLOSPWV
clear,clc,close all
%--------产生跳频信号
fs=5*10^3;   %采样频率4
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
sig=awgn(sig,20,'measured');%加高斯白噪声，信噪比为10dB
A1=0.2;A2=0.6;  %调制信号幅度
f1=0.8e3;f2=2.2e3;%调制信号频率
t1= [0:N-1]/fs;%采样时间
xt=A1*cos(2*pi*f1*t1);%调制信号+A2*cos(2*pi*f2*t1)
sig=sig+xt;                     %%混合AM信号和跳频信号



figure();
plot(sig);
title('调频信号');

figure();
x = sig';  
x = hilbert(x);  %       得到解析信号
N = 2^nextpow2(length(x)); 
f = 0 :fs/length(x): (fs - fs/length(x));  
figure(); 
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
                                       
figure(); 
mesh(abs(TFR1));   %画三维图  
title('STFT-时频分布图 汉明窗（127）'); 
xlabel('Time'),ylabel('Frequency'),zlabel('Amplitude'); 
 
figure(); 
contour(T,F(1:N/2),abs(TFR1(1:N/2,:)));  %画等高线 
title('STFT contour 汉明窗（127）'); 
xlabel('Time'),ylabel('Normalized frequency'); 


figure(); 
imagesc(T,F,abs(TFR1));   %imagesc(A) 将矩阵A中的元素数值按大小转化为不同颜色， 
                          %并在坐标轴对应位置处以这种颜色染色 
title('STFT imagesc 汉明窗（127）'); 

%% spwvd时频分布
% figure();
% signal=hilbert(sig);
% [tfr,t,f]=tfrspwv(signal');
% axis([0 N 0 0.5]) 
% imagesc(t,f,abs(tfr));
% title('SPWVD时频图');
% xlabel('时域采样点');
% ylabel('频域采样点');
S = TFR1;
[St,Ft]=max(S); %a row vector containing the maximum element from each column. S是行向量,对应每一列的max value，F是对应max index
St=abs(St);
Ft=abs(Ft);
figure();
plot(St);
title('跳频信号峰值包络y(n)');
xlabel('时间/us');
ylabel('峰值幅值');

figure(44)
plot(St);

figure();
F1=T*fs/length(T);
St = St - mean(St);
plot(F1,abs((fft(St))));
% axis([0,2e5,0,2.5e4]);
title('y(n)的FFT谱');
xlabel('频率/Hz');
ylabel('FFT幅度');

figure(55)
plot(St);%T*ts*1e6,


F=Ft/N*fs/1e6-fs/N*1e-6;
figure();
plot(abs(F));%T*ts*1e6,
title('时频脊线');
xlabel('时间/us');
ylabel('频率/MHz');


T_hop=diff(Ft);
figure();
T1=1:length(T_hop);
plot(abs(T_hop)); %T1*ts*1e6,
title('跳频时刻');
xlabel('时间/us');
ylabel('幅值');

%对时频脊线一阶差分
dn = abs(diff(Ft));
%找出去毛刺门限
d_sortn = sort(dn); %0 毛刺序列 脉冲序列 
d_divn = zeros(1,length(d_sortn));
% NaN  无穷大 接近1 >1的序列
for L=1:length(d_sortn)-1; 
    d_divn(L)=d_sortn(L+1)/d_sortn(L); 
end;

for i=1:length(d_divn); 
    if(d_divn(i)== Inf) 
        continue;    %第一个最大值为零值和毛刺的分界
    else if(d_divn(i)>5)
            break;   %毛刺和序列的分界
        end;
    end; 
end;

%序列dsortn中该位置对应的值即为所求的门限值
th4 = d_sortn(i)
d1n = zeros(1,length(dn));%根据门限th4去除了毛刺序列
for j=1:length(dn)
        if dn(j) > th4
           d1n(j) = dn(j);
       else
           d1n(j) = 0;
       end
end
figure();
plot(d1n); 
%% tftistft
% [X,T] = tfristft(TFR1,T,H);
% figure()
% plot(T,X);


% %% 功率谱
% b = sig;
% fs1 = fs;
% window=hamming(length(b)); %汉明窗
% figure();
% % Hs=spectrum.periodogram('Hamming');
% % psd(Hs,data,'fs',fs);%%功率谱
% % [Pxx,F] = psd(Hs,data,fs,'twosided');
% nfft=2048;
% [Pxx,f]=periodogram(b,window,nfft,fs1); %周期图法
% Pow= (fs1/length(Pxx)) * sum(Pxx);      %平均功率
% plot(f,10*log10(Pxx));
% title('整段信号的功率谱');
% xlabel('Frequency');
% ylabel('Power/frequency(dB/Hz)');
% 
% 
% %%每一段数据的功率谱
% figure();
% b1=b(1:1024,1);     %分四段
% b2=b(1025:2048,1);
% b3=b(2049:3072,1);
% b4=b(3073:4096,1);
% % window1=boxcar(length(b1)); %矩形窗
% window1=hamming(length(b1)); %汉明窗
% subplot(221);
% nfft1=2048;
% [Pxx1,f1]=periodogram(b1,window1,nfft1,fs1); %周期图法
% plot(f1,10*log10(Pxx1));
% Pow1= (fs1/length(Pxx1)) * sum(Pxx1);
% subplot(222);
% [Pxx2,f2]=periodogram(b2,window1,nfft1,fs1); %直接法
% plot(f2,10*log10(Pxx2));
% subplot(223);
% [Pxx3,f3]=periodogram(b3,window1,nfft1,fs1); %直接法
% plot(f3,10*log10(Pxx3));
% subplot(224);
% [Pxx4,f4]=periodogram(b4,window1,nfft1,fs1); %直接法
% plot(f4,10*log10(Pxx4));
% 
% 
% 
% figure();%整段数据的平均功率谱
% Pxx7=(Pxx1.*Pxx2.*Pxx3.*Pxx4);
% Pow7= (fs1/length(Pxx7)) * sum(Pxx7);
% plot(f4,(10/4)*log10(Pxx7));
% title('整段数据的平均功率谱');
% xlabel('Frequency');
% ylabel('Power/frequency(dB/Hz)');
% 
% figure();%对消后的功率谱
% % Pxx8=(Pxx1-Pxx7)+(Pxx2-Pxx7)+(Pxx3-Pxx7)+(Pxx4-Pxx7);
%  %Pxx8=[(Pxx1/Pxx7);(Pxx2/Pxx7);(Pxx3/Pxx7);(Pxx4/Pxx7)];
% Pxx8=10*log10(Pxx)-(10/4)*log10(Pxx7);
% %Pxx8=[10*log10(Pxx);0;0;0]-[10*log10(Pxx7);10*log10(Pxx7);10*log10(Pxx7);10*log10(Pxx7)];Pxx8=Pxx8(1:2049,1);
%  
% Pow8= (fs1/length(Pxx8)) * sum(Pxx8);
% 
% plot(f,Pxx8);
% title('对消后的功率谱');
% xlabel('Frequency');
% ylabel('Power/frequency(dB/Hz)');
% yita=Pow/Pow8;%功率对消比


