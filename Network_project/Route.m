clc;

server = "lon.speedtest.clouvider.net";

TTL = 0;
for n = 1:255
    command = "ping -n 2 -i " + n + " "+ server;
    disp(command);
    [status,cmdout] = system(command);
    if ~ contains(cmdout,'TTL scaduto durante il passaggio')
        if ~ contains(cmdout,'Richiesta scaduta.')
            TTL = n; 
            break;
        end
    end
    disp("TTL scaduto durante il passaggio.");
end

disp("E' necessario attraversare: " + TTL + " Link");