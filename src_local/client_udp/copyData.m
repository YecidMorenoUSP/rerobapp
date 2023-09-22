clc
clearvars

!scp -r debian@192.168.0.232:/home/debian/out .
%%
f = "out/_threadControl.dat";

d = importdata(f,'\t');

idx = find(d.data(:,1)==0);

fprintf(1,"%d/%d\n",numel(idx),numel(d.data(:,1)));

d.data(idx,: ) = [ ] ;

data = {};

for idx = 1:numel(d.colheaders)
    data.(d.colheaders{idx}) = d.data(:,idx);
end

t = data.time;

figure(200), clf

subplot(221)
plot(t,data.knee_pos_in)
ylabel("position [u]")

subplot(222)
plot(t,data.knee_pos_out)
ylabel("position [u]")

subplot(223)
plot(t,data.knee_vel_in)
ylabel("velocity [u/s]")

subplot(224)
plot(t,data.knee_vel_out)
ylabel("velocity [u/s]")

for ax = findall(gcf,'type','axes')'
    axes(ax)
    hold on
    grid minor
    xlabel("Time [s]")
    xlim([t(1) t(end)])
    drawnow
end