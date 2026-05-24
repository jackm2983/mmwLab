% EE541 - Link Budget 
% 11 Feburary 2025 
 
close all; clear; clc; 
 
% constants 
c = 3e8; 
k = 1.380649e-23; % boltzman constant 
T = 293; % temp in Kelvin 
 
% system specifications 
f = 2.45e9; % center frequency 
lambda = c / f; 
B = 100e6; % bandwidth 
d = 6; % distance between antennas in meters 
Te = 150; 
 
% transmit side specification 
modulator_po_dBm = 1; 
modulator_nsd_dBmHz = -156; 
preamp_g_dB = 23.4; 
preamp_nf_dB = 2.8; 
pa_g_dB = 15.9; 
pa_nf_dB = 2.1; 
bpf_g_dB = -2; 
tx_antenna_dB = 15; 
 
% receive side specification 
rx_antenna_dB = 9.5; 
lna_g_dB = 16.3; 
lna_nf_dB = 0.8; 
alt_lna_g_dB = 17.3; 
alt_lna_nf_dB = 0.44; 
 
% modulator SNR 
modulator_ps_W = 10 ^ ((modulator_po_dBm-30)/10); 
modulator_nsd_WHz = 10 ^ ((modulator_nsd_dBmHz-30)/10); 
modulator_pn_W = modulator_nsd_WHz * B; 
modulator_SNR = modulator_ps_W/modulator_pn_W; 
modulator_SNR_dB = 10*log10(modulator_SNR); 
62 
 
disp("Power at Modulator (dBW)" + 10*log10(modulator_ps_W)) 
disp("Noise at Modulator (dBW)" + 10*log10(modulator_pn_W)) 
disp("SNR at Modulator (dB): " + modulator_SNR_dB); 
disp(" "); 
 
% tx chain 
preamp_g = 10^(preamp_g_dB/10); 
preamp_f = 10^(preamp_nf_dB/10); 
pa_g = 10^(pa_g_dB/10); 
pa_f = 10^(pa_nf_dB/10); 
bpf_g = 10^(bpf_g_dB/10); 
bpf_f = 1/bpf_g; 
f_at_tx_antenna = preamp_f + ((pa_f-1)/(preamp_g)) + ((bpf_f-1)/(preamp_g*pa_g)); 
SNR_at_tx_antenna = modulator_SNR / f_at_tx_antenna; 
p_at_tx_antenna = modulator_ps_W * preamp_g * pa_g * bpf_g; 
pn_at_tx_antenna = p_at_tx_antenna / SNR_at_tx_antenna; % get noise power at 
antenna 
disp("Power at Tx Antenna (dBW): " + 10*log10(p_at_tx_antenna)); 
disp("Noise Figure at Tx Antenna (dB): " + 10*log10(f_at_tx_antenna)); 
disp("SNR at Tx Antenna (dB): " + 10*log10(SNR_at_tx_antenna)); 
disp(" "); 
 
% at rx antenna 
tx_antenna_g = 10^(tx_antenna_dB/10); 
rx_antenna_g = 10^(rx_antenna_dB/10); 
p_at_rx_antenna = p_at_tx_antenna * tx_antenna_g * rx_antenna_g * 
((lambda/(4*pi*d))^2); 
%pn_at_rx_antenna = k*(T+Te)*B; % likely inaccurate 
pn_at_rx_antenna = pn_at_tx_antenna * tx_antenna_g * rx_antenna_g * 
((lambda/(4*pi*d))^2); % use Friis' with noise 
SNR_at_rx_antenna = p_at_rx_antenna / pn_at_rx_antenna; 
disp("Power at Rx Antenna (dBW): " + 10*log10(p_at_rx_antenna)); 
disp("Max SNR at Rx Antenna (dB): " + 10*log10(SNR_at_rx_antenna)); 
disp(" "); 
 
% rx chain 
lna_g = 10^(lna_g_dB/10); 
lna_f = 10^(lna_nf_dB/10); 
f_at_demod = lna_f + ((bpf_f-1)/(lna_g)); 
demodulator_SNR = SNR_at_rx_antenna / f_at_demod; 
p_at_demodulator = p_at_rx_antenna * lna_g * bpf_g; 
disp("Power at Demodulator (dBW): " + 10*log10(p_at_demodulator)); 
disp("Noise Figure at Demodulator (dB): " + 10*log10(f_at_demod)); 
disp("Max SNR at Demodulator (dB): " + 10*log10(demodulator_SNR));