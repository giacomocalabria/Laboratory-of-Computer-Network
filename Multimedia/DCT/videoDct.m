%% video dct basis

BS = dctmtx(8)';

figure;
bar3(BS);
v = VideoWriter('dct.avi');
open(v)
Theta = -45:45;
F(numel(Theta)) = struct('cdata',[],'colormap',[]);

for theta = Theta,
    set(gca,'View',[ theta   30] )
    drawnow
    
    writeVideo(v,getframe(gcf))
end

%fig = figure;
%movie(fig,F,1)

close(v)