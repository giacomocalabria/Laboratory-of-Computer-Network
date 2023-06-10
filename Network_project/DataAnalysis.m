close all;
clear all;
clc;

%% Set up the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 5);
% Specify delimiter of data
opts.Delimiter = ";";
% Specify column names and types
opts.VariableNames = ["L", "RTTmin", "RTTavg", "RTTmax", "RTTmdev"];
opts.VariableTypes = ["double", "double", "double", "double", "double"];
% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";
% Specify variable properties
opts = setvaropts(opts, "RTTmdev", "TrimNonNumeric", true);
opts = setvaropts(opts, "RTTmdev", "ThousandsSeparator", ",");
% Import the data
risttl = readtable("ris_rtt.txt", opts);
% Convert to output type
risttl = table2array(risttl);


%% Converts the packet size from byte to bits

risttl(:,1) = 8 * (risttl(:,1) + 28);

%% Compute graph
fig = figure;

% Graph rtt_min
subplot(2, 2, 1);
plot(risttl(:, 1), risttl(:, 2), 'o');
grid on;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT min (ms)','interpreter','latex');
title('RTT minimo','interpreter','latex');

% Graph rtt_avg
subplot(2, 2, 2);
plot(risttl(:, 1), risttl(:, 3), 'o');
grid on;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT avg (ms)','interpreter','latex');
title('RTT medio','interpreter','latex');

% Graph rtt_max
subplot(2, 2, 3);
plot(risttl(:, 1), risttl(:, 4), 'o');
grid on;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT max (ms)','interpreter','latex');
title('RTT massimo','interpreter','latex');

% Graph rtt_mdev
subplot(2, 2, 4);
plot(risttl(:, 1), risttl(:, 5), 'o');
grid on;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT mdev (ms)','interpreter','latex');
title('RTT deviazione standard','interpreter','latex');

%saveas(fig,"rtt all.png")

%% Compute troughtput

p = polyfit(risttl(:,1),risttl(:,2),1);

fig = figure;

plot(risttl(:,1),risttl(:,2),'o');
hold on;
grid on;
plot(risttl(:,1),p(1)*risttl(:,1)+p(2),'-','LineWidth',1.5);
hold off;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT min (ms)','interpreter','latex');
title('RTT minimo','interpreter','latex');

saveas(fig,"rtt fit.png");

disp("a = " + p(1) + " T = " + p(2));

n = 22;
R = n/p(1);
Rb = 2/p(1);

disp("R = " + R + " R_bttneck = " + Rb);
