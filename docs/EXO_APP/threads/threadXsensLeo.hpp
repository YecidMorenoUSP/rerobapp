#pragma once

#include <thread_exo.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#include <declarations_xsens.h>
#pragma GCC diagnostic pop
#include <LowPassFilter2p.h>
//#include <xsensLeoUtils.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>

#include <utils_plot.hpp>
#include <chrono>


#define OFFSET_US int(0.260 * 1000000)

typedef struct {
	float data[10];
} fromXsensLeo;

class ThreadXsensLeo : public ThreadType {

private:
	fromXsensLeo data;
	std::atomic<bool> plotting;
	PlotWindow pw;

	// Human joints euler angles:
	Eigen::Vector3f right_ankle_euler;
	Eigen::Vector3f right_knee_euler;  // mod 17062023
	Eigen::Vector3f left_ankle_euler;
	Eigen::Vector3f left_knee_euler;

	std::vector<Eigen::Vector4f> qASGD;
	std::vector<Eigen::Matrix4f> qPk;

	// Offset quaternions: remove arbitrary attitude on initialization
	std::vector<Eigen::Vector4f> qOffsets; // q12Off, q23Off, q45Off, q56Off
	std::vector<Eigen::Matrix3f> Rotate;

	std::chrono::time_point<std::chrono::steady_clock> t_begin;

public:

	ThreadXsensLeo() {
		pw = PlotWindow("Units [ u ]", "Time [ s ]", "Xsens Data");
		plotting = false;
	}

	float imus_data[N_DATA_IMU];
	float states_data[10];
	int offCount;


	void _setup() {

		((ThreadXsensRead*)XsensRead->threadType_)->sensorsSxens[0].required = true;
		((ThreadXsensRead*)XsensRead->threadType_)->sensorsSxens[1].required = true;
		((ThreadXsensRead*)XsensRead->threadType_)->sensorsSxens[2].required = true;
		((ThreadXsensRead*)XsensRead->threadType_)->sensorsSxens[3].required = true;

		using namespace std;
		using namespace Eigen;

		if (XsensRead->toRUNfromGUI == false)
			throw std::runtime_error("Precisa iniciar o thread XsensRead");


		for (size_t i = 0; i < int(N_DATA_IMU / 6); i++)
		{
			qOffsets.push_back(Eigen::Vector4f(1, 0, 0, 0));
			qASGD.push_back(Eigen::Vector4f(1, 0, 0, 0));
			qPk.push_back(Eigen::Matrix4f::Identity() * 0.01);
		}

		Eigen::Matrix3f R;

		R << 0, 0, 1,
			0, -1, 0,
			1, 0, 0;
		Rotate.push_back(R); // canela dir

		// R idem
		Rotate.push_back(R); // coxa dir

		R << 0, 0, 1,
			0, -1, 0,
			1, 0, 0;
		Rotate.push_back(R); // canela esq

		// R idem
		Rotate.push_back(R); // coxa esq

		imus_data[18] = { 0 };
		states_data[10] = { 0 };
		offCount = 0;

		{
			std::unique_lock<std::mutex> _(_mtx);
			pw.SCOPE.y_axis.minimum = -M_PI / 2.0;
			pw.SCOPE.y_axis.maximum = M_PI / 2.0;
			pw.SCOPE.x_axis.minimum = 0.0f;
			pw.SCOPE.x_axis.maximum = T_exec;
			pw.clearItems();
			pw.addItem("omega_p");
			pw.addItem("theta_p");
			pw.addItem("alpha_p");
		}

		t_begin = chrono::steady_clock::now();
	}

	void _firstLoop() {

	}

	void _cleanup() {

	}
	fromXsensRead imu_shared_data;
	void _loop() {

		using namespace std;
		using namespace Eigen;

		XsensRead->getData(&imu_shared_data);
		memcpy(imus_data, imu_shared_data.imus_data, sizeof(imus_data));

		static Vector3f imus_gyro;
		static Vector3f imus_acc;

		for (size_t i = 0; i < (N_DATA_IMU / 6); i++)
		{
			Vector3f imu_gyro_vector(imus_data[6 * i + 0], imus_data[6 * i + 1], imus_data[6 * i + 2]);
			Vector3f imus_acc_vector(imus_data[6 * i + 3], imus_data[6 * i + 4], imus_data[6 * i + 5]);

			imus_gyro = Rotate[i] * imu_gyro_vector;
			imus_acc  = Rotate[i] * imus_acc_vector;

			qASGDKalman(qASGD[i], qPk[i], this->_Ts, imus_acc, imus_gyro);
		}

		// Compute joints euler angles:
		// Right Hand Rule:
		right_knee_euler = quatDelta2Euler(qASGD[0], qASGD[1]); // IMU1 - IMU2
		// Left  Hand Rule:
		left_knee_euler = quatDelta2Euler(qASGD[3], qASGD[2]); // IMU4 - IMU3



		// Remove arbitrary IMU attitude:
		int elapsedTime = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - t_begin).count();
		// Time average:
		if (elapsedTime < OFFSET_US) //
		{
			float incrmnt = (this->_Ts * 1000000) / OFFSET_US;
			// Right Hand Rule:
			qOffsets[0] += incrmnt * qDelta(qASGD[0], qASGD[1]);
			// Left  Hand Rule:
			qOffsets[1] += incrmnt * qDelta(qASGD[3], qASGD[2]);

		}
		if (elapsedTime < (OFFSET_US + static_cast<long long>(7000000 * this->_Ts)) && elapsedTime >= OFFSET_US)
		{
			for (size_t i = 0; i < qOffsets.size(); i++)
			{
				qOffsets[i].normalize();
			}

		}
		else {   
			// Compensate arbitrary IMU attitude:
			// Right Hand Rule:
			right_knee_euler = quatDelta2Euler(qASGD[0], qDelta(qASGD[1],qOffsets[0]));
			// Left  Hand Rule:
			left_knee_euler = quatDelta2Euler(qDelta(qASGD[3], qOffsets[1]), qASGD[2]);
		}

		states_data[0] = -right_knee_euler(0);  // knee_pos
		states_data[1] = 0;
		states_data[2] = 0;
		states_data[3] = -left_knee_euler(0); // ankle_pos
		states_data[4] = 0;
		states_data[5] = 0;
		states_data[6] = 0;
		states_data[7] = 0;
		states_data[8] = 0;
		states_data[9] = 0;

		{ // sessao critica
			unique_lock<mutex> _(_mtx);
			memcpy(data.data, states_data, sizeof(states_data));
		} // fim da sessao critica

		_datalog[time_index][0] = timer->get_current_time_f();
		for (int i = 0; i < 10; i++) {
			_datalog[time_index][i + 1] = states_data[i];
		}

		if (true) {
			{
				std::unique_lock<std::mutex> _(_mtx);
				pw.items[0].data.push_back(ImVec2((float)_datalog[time_index][0], 180.0f/M_PI* (float)_datalog[time_index][1])); // joelho dir
				pw.items[1].data.push_back(ImVec2((float)_datalog[time_index][0], 180.0f/M_PI* (float)_datalog[time_index][4])); // joelho esq
			}
		}
	}

	//bool cbPlot = false;
	bool cbPlot = true;
	void _updateGUI() {
		ImGui::Begin(_name.c_str());

		ImGui::Checkbox("Graph", &cbPlot);
		plotting = cbPlot;
		if (plotting) {
			{
				std::unique_lock<std::mutex> _(_mtx);
				pw.showNewWindow();
			}
		}

		ImGui::End();
	}

	void getData(void* _data) {
		_mtx.lock();
		memcpy(_data, &data, sizeof(fromXsensLeo));
		_mtx.unlock();
	}

private:

	void removeYaw(Eigen::Vector4f* quat)
	{
		float q0 = (*quat)(0);
		float q1 = (*quat)(1);
		float q2 = (*quat)(2);
		float q3 = (*quat)(3);
		float yaw = atan2f(2 * q1 * q2 + 2 * q0 * q3, \
			q1 * q1 + q0 * q0 - q3 * q3 - q2 * q2);
		Eigen::Matrix4f  Qy = Eigen::Matrix4f::Identity() * cosf(-yaw / 2);
		Qy(0, 3) = -sinf(-yaw / 2);
		Qy(1, 2) = Qy(0, 3);
		Qy(2, 1) = -Qy(0, 3);
		Qy(3, 0) = -Qy(0, 3);
		*quat = Qy * (*quat);
		quat->normalize();
	}

	Eigen::Vector4f qDelta(const Eigen::Vector4f quat_r, const Eigen::Vector4f quat_m)
	{
		float qr0 = quat_r(0);
		float qr1 = quat_r(1);
		float qr2 = quat_r(2);
		float qr3 = quat_r(3);
		// q_m conjugate (*q_m):
		float qm0 = quat_m(0);
		float qm1 = -quat_m(1);
		float qm2 = -quat_m(2);
		float qm3 = -quat_m(3);

		Eigen::Vector4f q;
		// quaternion product: q_r x *q_m:
		q(0) = qr0 * qm0 - qr1 * qm1 - qr2 * qm2 - qr3 * qm3;
		q(1) = qr0 * qm1 + qr1 * qm0 + qr2 * qm3 - qr3 * qm2;
		q(2) = qr0 * qm2 - qr1 * qm3 + qr2 * qm0 + qr3 * qm1;
		q(3) = qr0 * qm3 + qr1 * qm2 - qr2 * qm1 + qr3 * qm0;
		return q;
	}

	Eigen::Vector3f quatDelta2Euler(const Eigen::Vector4f quat_r, const Eigen::Vector4f quat_m)
	{
		using namespace Eigen;
		// quaternion product: q_r x *q_m:
		Vector4f q = qDelta(quat_r, quat_m);
		Vector3f euler;
		euler(0) = atan2f(2 * q(2) * q(3) + 2 * q(0) * q(1), q(3) * q(3) - q(2) * q(2) - q(1) * q(1) + q(0) * q(0));
		euler(1) = -asinf(2 * q(1) * q(3) - 2 * q(0) * q(2));
		euler(2) = atan2f(2 * q(1) * q(2) + 2 * q(0) * q(3), q(1) * q(1) + q(0) * q(0) - q(3) * q(3) - q(2) * q(2));
		return euler;
	}

	Eigen::Vector3f RelVector(const Eigen::Vector4f rel_quat, const Eigen::Vector3f vec_r, const Eigen::Vector3f vec_m)
	{
		return  (vec_m - Eigen::Quaternionf(rel_quat).toRotationMatrix() * vec_r);
	}

	Eigen::Vector3f RelAngAcc(const Eigen::Vector4f rel_quat, const Eigen::Vector3f rel_ang_vel, const Eigen::Vector3f rel_linear_acc)
	{
		using namespace Eigen;
		Matrix3f Rot = Quaternionf(rel_quat).toRotationMatrix();

		Vector3f linAccFrame2 = Rot.transpose() * rel_linear_acc;
		Vector3f angVelFrame2 = Rot.transpose() * rel_ang_vel;

		float omg_x = angVelFrame2(0);
		float omg_y = angVelFrame2(1);
		float omg_z = angVelFrame2(2);
		float acc_x = linAccFrame2(0);
		float acc_y = linAccFrame2(1);
		float acc_z = linAccFrame2(2);
		float alpha_x, alpha_y, alpha_z;

		// using centriptal and radial acc decompositon:
		float norm_zy = sqrt(acc_z * acc_z + acc_y * acc_y);
		float phi = atan2f(-acc_y, acc_z); // -y devido a orientacao adotada das imus nas pernas
		alpha_x = norm_zy * sinf(phi); // aproximando R=1

		float norm_xz = sqrt(acc_x * acc_x + acc_z * acc_z);
		phi = atan2f(acc_x, -acc_z);
		alpha_y = norm_xz * sinf(phi); // aproximando R=1

		float norm_xy = sqrt(acc_x * acc_x + acc_y * acc_y);
		phi = atan2f(acc_y, -acc_x);
		alpha_z = norm_xy * sinf(phi); // aproximando R=1

		return Vector3f(alpha_x, alpha_y, alpha_z);
	}


	void qASGDKalman(Eigen::Vector4f& qk, Eigen::Matrix4f& Pk, float Ts, Eigen::Vector3f acc, Eigen::Vector3f gyro)
	{
		using namespace Eigen;
		// Declarations
		float q0 = qk[0];
		float q1 = qk[1];
		float q2 = qk[2];
		float q3 = qk[3];
		const Matrix4f H = Matrix4f::Identity();
		const Matrix4f R = Matrix4f::Identity() * 2.5e-5;

		FullPivLU<Matrix4f> Ck; // Covariance Matrix
		Matrix4f Qk = Matrix4f::Identity() * 5.476e-6; // Usar Eq. 19...
		Matrix4f KG; // Kalman Gain
		Vector3f F_obj;
		Vector4f GradF;
		Vector4f z_k;
		Vector3f Zc;
		Matrix4f OmG;
		Matrix4f Psi;
		Matrix<float, 3, 4> Jq;
		Matrix<float, 4, 3> Xi;
		float omg_norm = gyro.norm();
		float miZero(0.3600f);
		float Beta(10.00f);
		float mi(0);

		// ASGD iteration:
		Zc << 2 * (q1 * q3 - q0 * q2),
			2 * (q2 * q3 + q0 * q1),
			(q0 * q0 - q1 * q1 - q2 * q2 - q3 * q3);
		F_obj = Zc - acc.normalized(); // Eq.23

		Jq << -2 * q2, 2 * q3, -2 * q0, 2 * q1,
			2 * q1, 2 * q0, 2 * q3, 2 * q2,
			2 * q0, -2 * q1, -2 * q2, 2 * q3;

		GradF = Jq.transpose() * F_obj; // Eq.25

		omg_norm = gyro.norm();
		mi = miZero + Beta * Ts * omg_norm; // Eq.29

		z_k = qk - mi * GradF.normalized(); // Eq.24
		z_k.normalize();

		OmG << 0, -gyro(0), -gyro(1), -gyro(2),
			gyro(0), 0, gyro(2), -gyro(1),
			gyro(1), -gyro(2), 0, gyro(0),
			gyro(2), gyro(1), -gyro(0), 0;
		OmG = 0.5 * OmG;

		Psi = (1 - ((omg_norm * Ts) * (omg_norm * Ts)) / 8) * H + 0.5 * Ts * OmG; // Using H as 'I_44'

		// Process noise covariance update (Eq. 19):
		Xi << q0, q3, -q2, -q3, q0, q1, q2, -q1, q0, -q1, -q2, -q3;
		Qk = 0.5 * Ts * Xi * (Matrix3f::Identity() * 5.476e-6) * Xi.transpose();
		// Projection:
		qk = Psi * qk;
		Pk = Psi * Pk * Psi.transpose() + Qk;
		// Kalman Gain (H is Identity)
		Ck = FullPivLU<Matrix4f>(Pk + R);
		if (Ck.isInvertible())
			KG = Pk * Ck.inverse();
		// Update (H is Identity)
		qk = qk + KG * (z_k - qk);
		Pk = (Matrix4f::Identity() - KG) * Pk;
		qk.normalize();

		// Rotate the quaternion by a quaternion with -(yaw):
		removeYaw(&qk);
	}
};
