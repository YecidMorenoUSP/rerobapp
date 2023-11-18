function client_udp

clc
clearvars
close all


makeUI()

if ~isempty(timerfindall)
    stop(timerfindall)
end



global s
s = udpport('datagram');
configureCallback(s,"datagram",1,@onData)

t = timer("StartDelay",0,"TimerFcn",@(~,~)onTimer,"ExecutionMode","fixedRate","Period",2);
start(t)


end

function onTimer(~,~)
    global s
%     write(s,[ uint8('ACK') 10 ],"uint8","127.0.0.1",1515);
%     write(s,[ uint8('ACK') 10 ],"uint8","192.168.6.2",1515);
    write(s,[ uint8('ACK') 10 ],"uint8","192.168.0.232",1515);
end

function updateLine(line,time,value)

    if(time == -1)
        line.XData = nan;
        line.YData = nan;
        return
    end

    if isnan(line.XData)
        line.XData = time;
        line.YData = value;

    elseif (-line.XData(1)+line.XData(end)) <= 5
        line.XData = [line.XData time];
        line.YData = [line.YData value];
        if length(line.XData) >= 10
            set(line.Parent,'XLim',[line.XData(1) line.XData(1)+5])
        end

    else
        line.XData = [line.XData(2:end) time];
        line.YData = [line.YData(2:end) value];
        set(line.Parent,'XLim',[line.XData(1) line.XData(end)])
    end       

end

function onData(~,~)

    global s VARS_STREAM UI

    SHM_VARS_STREAM_KEY = hex2dec('1010');

    s_read = s.read(1);
    
    key = typecast(uint8(s_read.Data(1:2)),'int16');    
   
    persistent t_last

    

     if key == SHM_VARS_STREAM_KEY
        VARS_STREAM = shm_vars_stream_cast(s_read.Data);

        if isempty(t_last)
            t_last = VARS_STREAM.t_s;
        else
            if(t_last == VARS_STREAM.t_s)
                return;
            end

            if t_last > VARS_STREAM.t_s
                VARS_STREAM.t_s = -1;
            end
            
            t_last = VARS_STREAM.t_s;
        end
        
        updateLine(UI.r_k_theta_in,VARS_STREAM.t_s,VARS_STREAM.exo_knee_rigth_pos_in);
        updateLine(UI.r_k_theta_out,VARS_STREAM.t_s,VARS_STREAM.exo_knee_rigth_pos_out);

        updateLine(UI.r_k_omega_in,VARS_STREAM.t_s,VARS_STREAM.exo_knee_rigth_vel_in);
        updateLine(UI.r_k_omega_out,VARS_STREAM.t_s,VARS_STREAM.exo_knee_rigth_vel_out);
        
        updateLine(UI.lines.exo_hip_rigth_pos_out,VARS_STREAM.t_s,VARS_STREAM.exo_hip_rigth_pos_out);
       


     end   

end

function onClose(obj,~)
    
    global s
    if ~isempty(timerfindall)
        stop(timerfindall)
    end
    configureCallback(s,"off");
    flush(s,"output");
    clear s

    delete(obj)
    
end

function makeUI

    global UI 

    UI = {};
    
    fig = figure(100);
    fig.Position(3:4) = [800 500];
    fig.CloseRequestFcn = @onClose;
    clf
    
    annotation("textbox","Position",[0,.9,1,.1], ...
        "BackgroundColor","w",'String',"ReRobAPP [ExoTAO]", ...
        "HorizontalAlignment","center","VerticalAlignment","middle", ...
        "FontSize",18,"EdgeColor","none");

    aximg = axes(Parent=fig,Position=[-.1,0,.5,1]);
    [img, map] = imread("exo.jpg");
    imshow(img,map,Parent=aximg,Interpolation="bilinear",Border="tight")
    
    
    UI.axpos = axes(Position=[.45,.5,.5,.3]);
    set(UI.axpos,'nextplot','add','xminorgrid','on','yminorgrid','on', ...
        "Title",text(0,0,"Position"))
    legend off
    UI.axvel = axes(Position=[.45,.1,.5,.3]);
    set(UI.axvel,'nextplot','add','xminorgrid','on','yminorgrid','on', ...
        "Title",text(0,0,"Velocity"))

    
    UI.r_k_theta_in = plot(nan,nan,"Parent",UI.axpos,'DisplayName',"\theta_k^r");
    UI.r_k_theta_out = plot(nan,nan,"Parent",UI.axpos,'DisplayName',"\theta_k^r");

    UI.r_k_omega_in = plot(nan,nan,"Parent",UI.axvel,'DisplayName',"\omega_k^r");
    UI.r_k_omega_out = plot(nan,nan,"Parent",UI.axvel,'DisplayName',"\omega_k^r");
    

     
    vars_read = fieldnames(shm_vars_stream_cast(zeros(1,10000)));
    assignin("base","vars_read",vars_read);
    for idx = 2:numel(vars_read)
        
        
        if contains(vars_read{2},"pos") == 1
            curAx = 'UI.axpos';
        else
            curAx = 'UI.axvel';
        end

        eval(sprintf("UI.lines.('%s') = plot(nan,nan,'Parent',%s,'DisplayName','%s');",vars_read{idx},curAx,vars_read{idx}));
    end

    assignin("base","fig",fig)       
end

% 

