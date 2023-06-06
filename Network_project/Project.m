close all;
clear all;
clc;

%% Set up the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 5);

% Specify range and delimiter
opts.DataLines = [1, Inf];
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
risttl = readtable("ris_rtt3.txt", opts);

% Convert to output type
risttl = table2array(risttl);

% Clear temporary variables
clear opts

risttl(:,1) = 8 * (risttl(:,1) + 28);

%% Crea i grafici
figure;

% Grafico rtt_min
subplot(2, 2, 1);
plot(risttl(:, 1), risttl(:, 2), 'o');
grid on;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT min (ms)','interpreter','latex');
title('RTT minimo','interpreter','latex');

% Grafico rtt_avg
subplot(2, 2, 2);
plot(risttl(:, 1), risttl(:, 3), 'o');
grid on;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT avg (ms)','interpreter','latex');
title('RTT medio','interpreter','latex');

% Grafico rtt_max
subplot(2, 2, 3);
plot(risttl(:, 1), risttl(:, 4), 'o');
grid on;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT max (ms)','interpreter','latex');
title('RTT massimo','interpreter','latex');

% Grafico rtt_mdev
subplot(2, 2, 4);
plot(risttl(:, 1), risttl(:, 5), 'o');
grid on;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT mdev (ms)','interpreter','latex');
title('RTT deviazione standard','interpreter','latex');

p = polyfit(risttl(:,1),risttl(:,2),1);

disp("a = " + p(1) + " T = " + p(2));

figure;

plot(risttl(:,1),risttl(:,2),'o');
hold on;
grid on;
plot(risttl(:,1),p(1)*risttl(:,1)+p(2),'-','LineWidth',1.5);
hold off;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT min (ms)','interpreter','latex');
title('RTT minimo','interpreter','latex');
