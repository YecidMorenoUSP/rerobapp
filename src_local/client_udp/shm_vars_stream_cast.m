function OUT = shm_vars_stream_cast(data_in)

    data = typecast(uint8(data_in(5:end)),'single');

    OUT = {};

    OUT.t_s = data(1);
    OUT.exo_hip_rigth_pos_in = data(2);
    OUT.exo_hip_rigth_vel_in = data(3);
    OUT.exo_hip_rigth_acc_in = data(4);
    OUT.exo_hip_rigth_pos_out = data(5);
    OUT.exo_hip_rigth_vel_out = data(6);
    OUT.exo_hip_rigth_acc_out = data(7);
    OUT.exo_knee_rigth_pos_in = data(8);
    OUT.exo_knee_rigth_vel_in = data(9);
    OUT.exo_knee_rigth_acc_in = data(10);
    OUT.exo_knee_rigth_pos_out = data(11);
    OUT.exo_knee_rigth_vel_out = data(12);
    OUT.exo_knee_rigth_acc_out = data(13);
    OUT.exo_ankle_rigth_pos_in = data(14);
    OUT.exo_ankle_rigth_vel_in = data(15);
    OUT.exo_ankle_rigth_acc_in = data(16);
    OUT.exo_ankle_rigth_pos_out = data(17);
    OUT.exo_ankle_rigth_vel_out = data(18);
    OUT.exo_ankle_rigth_acc_out = data(19);
    OUT.exo_hip_left_pos_in = data(20);
    OUT.exo_hip_left_vel_in = data(21);
    OUT.exo_hip_left_acc_in = data(22);
    OUT.exo_hip_left_pos_out = data(23);
    OUT.exo_hip_left_vel_out = data(24);
    OUT.exo_hip_left_acc_out = data(25);
    OUT.exo_knee_left_pos_in = data(26);
    OUT.exo_knee_left_vel_in = data(27);
    OUT.exo_knee_left_acc_in = data(28);
    OUT.exo_knee_left_pos_out = data(29);
    OUT.exo_knee_left_vel_out = data(30);
    OUT.exo_knee_left_acc_out = data(31);
    OUT.exo_ankle_left_pos_in = data(32);
    OUT.exo_ankle_left_vel_in = data(33);
    OUT.exo_ankle_left_acc_in = data(34);
    OUT.exo_ankle_left_pos_out = data(35);
    OUT.exo_ankle_left_vel_out = data(36);
    OUT.exo_ankle_left_acc_out = data(37);

end