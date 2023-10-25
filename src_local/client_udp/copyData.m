clc
clearvars

!scp -r debian@192.168.0.232:/home/debian/out .
%%
f = "out/file_Knee_L.dat";

d = importdata(f,'\t');

idx = find(d.data(:,1)==0);

fprintf(1,"%d/%d\n",numel(idx),numel(d.data(:,1)));

d.data(idx,: ) = [ ] ;

data = {};

for idx = 1:numel(d.colheaders)
    data.(d.colheaders{idx}) = d.data(:,idx);
end

t = data.time;
tau_cal = (data.knee_pos_in/150 - data.knee_pos_out)*104;

figure(200), clf

subplot(321)
hold on
plot(t,data.knee_pos_in)
plot(t,data.knee_pos_out*150)
ylabel("position [u]")

subplot(322)
plot(t,data.knee_pos_out)
ylabel("position [u]")

subplot(323)
hold on
plot(t,data.knee_vel_d)
plot(t,data.knee_vel_in)
legend("omega_d","omega")
ylabel("velocity [u/s]")

subplot(324)
plot(t,data.knee_vel_out)
ylabel("velocity [u/s]")

subplot(325)
hold on
plot(t,data.knee_tau_d)
plot(t,data.knee_tau_l)
legend("tau_d","tau")
ylabel("tau [u/s]")

for ax = findall(gcf,'type','axes')'
    axes(ax)
    hold on
    grid minor
    xlabel("Time [s]")
    xlim([t(1) t(end)])
    drawnow
end