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
risttl = readtable("ris_rtt.txt", opts);

% Convert to output type
risttl = table2array(risttl);

% Clear temporary variables
clear opts

risttl(:,1) = 8 * (risttl(:,1) + 28);

p = polyfit(risttl(:,1),risttl(:,2),1);

disp("a = " + p(1) + " T = " + p(2));

figure;

plot(risttl(:,1),risttl(:,2),'o');
hold on;
plot(risttl(:,1),p(1)*risttl(:,1)+p(2),'-');
hold off;
xlabel('L (pkt size) - bits','interpreter','latex');
ylabel('RTT min (ms)','interpreter','latex');
title('RTT minimo','interpreter','latex');