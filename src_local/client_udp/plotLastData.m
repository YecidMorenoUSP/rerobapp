function plotLastData(Nfigure,fileName)

    global UILast

    

    f = fileName;    
    d = importdata(f,'\t');    
    idx = find(d.data(:,1)==0);

    fprintf(1,"%d/%d\n",numel(idx),numel(d.data(:,1)));
    d.data(idx,: ) = [ ] ;
    data = {};
    
    for idx = 1:numel(d.colheaders)
        data.(d.colheaders{idx}) = d.data(:,idx);
    end
    
    t = data.time;
    tau_cal = (data.pos_in/150 - data.pos_out)*104;
    
    fig = figure(Nfigure); clf;
    UILast.fig = fig;
    set(fig,'units','normalized','outerposition',[0 0 1 1])
    
    subplot(321)
    hold on
    plot(t,data.pos_in)
    ylabel("position [u]")
    
    subplot(322)
    plot(t,data.pos_out)
    ylabel("position [u]")
    
    subplot(323)
    hold on
    plot(t,data.vel_d)
    plot(t,data.vel_in)
    legend("omega_d","omega")
    ylabel("velocity [u/s]")
    
    subplot(324)
    plot(t,data.vel_out)
    ylabel("velocity [u/s]")
    
    subplot(325)
    hold on
    plot(t,data.tau_d)
    plot(t,data.tau_l)
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

    assignin("base","UILast",UILast);

end

