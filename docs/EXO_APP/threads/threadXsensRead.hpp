#pragma once

#include <thread_exo.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#include <declarations_xsens.h>
#pragma GCC diagnostic pop
#include <LowPassFilter2p.h>

#include <utils_plot.hpp>


#define N_DATA_IMU 6*4

typedef struct{
    float imus_data[N_DATA_IMU];
} fromXsensRead;

typedef struct {

} XsensReadHandler;

typedef struct SensorSxens_s{
    std::string ID;
    bool active;
    bool required;
    SensorSxens_s(std::string _id) {
        ID = _id;
    }
} SensorSxens;

class ThreadXsensRead: public ThreadType{
    private:
        fromXsensRead data;
        int N_IMU ;

        std::vector<std::string> imus_names;
    public:       
      
        std::atomic<bool> plotting;
        std::vector<SensorSxens> sensorsSxens;
        PlotWindow pw;

        ThreadXsensRead(){
            pw = PlotWindow("Units [ u ]","Time [ s ]","Xsens Data");
            plotting = false;

            imus_names.push_back("00B412DF"); // IMU1
            imus_names.push_back("00B410D2"); // IMU2
            imus_names.push_back("00B41244"); // IMU3
            imus_names.push_back("00B4108C"); // IMU4

            sensorsSxens.clear();
            sensorsSxens.push_back(SensorSxens(imus_names[0]));
            sensorsSxens.push_back(SensorSxens(imus_names[1]));
            sensorsSxens.push_back(SensorSxens(imus_names[2]));
            sensorsSxens.push_back(SensorSxens(imus_names[3]));
            //sensorsSxens.push_back(SensorSxens("00B412CC"));
            for (int idx = 0; idx < sensorsSxens.size(); idx++) {
                sensorsSxens[idx].active = false;
                sensorsSxens[idx].required = false;
            }

            N_IMU = sensorsSxens.size();
        }

        XsDevicePtr wirelessMasterDevice = NULL;
        XsControl *control = NULL;
        XsPortInfoArray detectedDevices;
        XsPortInfoArray::const_iterator wirelessMasterPort;
        WirelessMasterCallback wirelessMasterCallback;
        XsDevicePtrArray mtwDevices, mtwDevicesOrdered;
        XsDeviceIdArray allDeviceIds;
        XsDeviceIdArray mtwDeviceIds;
        int desiredIMUs = 0;
        
        LowPassFilter2pFloat imu_filters[N_DATA_IMU];
        float imus_vals[N_DATA_IMU];
        std::vector<MtwCallback *> mtwCallbacks; 
        std::vector<XsEuler>  eulerData;
        std::vector<XsVector> accData;
        std::vector<XsVector> gyroData;
        std::vector<XsVector> magData;

        const int desiredUpdateRate = 75;
        const int desiredRadioChannel = 25;

        void _setup(){
            // throw "ERROR";
            using namespace std;
            // vector<int> imu_headers(4);
            
            for (int i = 0; i < (N_DATA_IMU); i++) imus_vals[i] = 0;

            mtwCallbacks.clear();

            control = XsControl::construct();
            if (control == 0) throw "Failed to construct XsControl instance.";

            try{      
                detectedDevices = XsScanner::scanPorts();
                wirelessMasterPort = detectedDevices.begin();
                while (wirelessMasterPort != detectedDevices.end() && !wirelessMasterPort->deviceId().isWirelessMaster()){
                    ++wirelessMasterPort;
                    cout << "Wireless master found @ " << *wirelessMasterPort << endl;
                }
                if (wirelessMasterPort == detectedDevices.end()){
                    throw runtime_error("No wireless masters found");
                }
                cout << "Wireless master found @ " << *wirelessMasterPort << endl;

                if (!control->openPort(wirelessMasterPort->portName().toStdString(), wirelessMasterPort->baudrate())){
                    ostringstream error;
                    error << "Failed to open port " << *wirelessMasterPort;
                    throw runtime_error(error.str());
                } 

                wirelessMasterDevice = control->device(wirelessMasterPort->deviceId());
                if (wirelessMasterDevice == 0){
                    ostringstream error;
                    error << "Failed to construct XsDevice instance: " << *wirelessMasterPort;
                    throw runtime_error(error.str());
                }
                if (!wirelessMasterDevice->gotoConfig()){
                    ostringstream error;
                    error << "Failed to goto config mode: " << *wirelessMasterDevice;
                    throw runtime_error(error.str());
                }
                wirelessMasterDevice->addCallbackHandler(&wirelessMasterCallback);

                const XsIntArray supportedUpdateRates = wirelessMasterDevice->supportedUpdateRates();
                const int newUpdateRate = findClosestUpdateRate(supportedUpdateRates, desiredUpdateRate);

                if (!wirelessMasterDevice->setUpdateRate(newUpdateRate)){
                    ostringstream error;
                    error << "Failed to set update rate: " << *wirelessMasterDevice;
                    throw runtime_error(error.str());
                }
                if (wirelessMasterDevice->isRadioEnabled()){
                    if (!wirelessMasterDevice->disableRadio()){
                        ostringstream error;
                        error << "Failed to disable radio channel: " << *wirelessMasterDevice;
                        throw runtime_error(error.str());
                    }
                }
                if (!wirelessMasterDevice->enableRadio(desiredRadioChannel)){
                    ostringstream error;
                    error << "Failed to set radio channel: " << *wirelessMasterDevice;
                    throw runtime_error(error.str());
                }

                cout << "Waiting for MTW to wirelessly connect..." << endl;
                size_t connectedMTWCount = wirelessMasterCallback.getWirelessMTWs().size();
                bool quitOnMTw = false;
                bool waitForConnections = true;
                int countIMUs = 0;


                
                
                desiredIMUs = 0;
                for (int idx = 0; idx < sensorsSxens.size(); idx++) {
                    sensorsSxens[idx].active = false;
                    desiredIMUs += (int)sensorsSxens[idx].required;
                    if(sensorsSxens[idx].required)
                        std::cout << "\n Desired : " << sensorsSxens[idx].ID << "\n";
                }
                mtwDevices.clear();
                mtwDevices.resize(N_IMU);
               

                for(int countIT = 0 ; countIT < 300 ; countIT++){
                
                    XsTime::msleep(100);

                    while (true)
                    {
                        size_t nextCount = wirelessMasterCallback.getWirelessMTWs().size();
                        
                        if (nextCount != connectedMTWCount){
                            cout << "Number of connected MTWs: " << nextCount <<endl;
                            connectedMTWCount = nextCount;
                            
                        }else break;

                        XsDeviceIdArray allDeviceIds = control->deviceIds();
                        
                        countIMUs = 0;
                        for (XsDeviceIdArray::const_iterator i = allDeviceIds.begin(); i != allDeviceIds.end(); ++i) {
                            
                            if (!i->isMtw()) continue;


                            XsDevicePtr mtwDevice = control->device(*i);

                            std::string IDD = mtwDevice->deviceId().toString().toStdString();
                            std::cout << "\n _setup ID: " << IDD << "\n";
                            for (int idx = 0; idx < sensorsSxens.size(); idx++) {
                                
                                if (!sensorsSxens[idx].required) continue;
                                if (sensorsSxens[idx].ID == IDD) {
                                    sensorsSxens[idx].active = true;
                                    mtwDevices[idx] = (mtwDevice);
                                    countIMUs++;
                                    std::cout << "\nADDED\n";
                                    
                                }
                            }

                        }
                    }
                   

                    if(countIMUs >= desiredIMUs) break;
                };
                
                int fondIMUS = 0;
                for (int idx = 0; idx < sensorsSxens.size(); idx++) {
                    if (sensorsSxens[idx].required == sensorsSxens[idx].active && sensorsSxens[idx].required == true)
                        fondIMUS++;
                }
                std::cout << "NUMMM >> " << mtwDevices.size() << " Desired:" << desiredIMUs << "  count : " << countIMUs << "  found : " << fondIMUS << "\n";
                if(fondIMUS != desiredIMUs) throw (std::runtime_error("Faltan IMUs"));


                /*XsDeviceIdArray allDeviceIds = control->deviceIds();
                for (XsDeviceIdArray::const_iterator i = allDeviceIds.begin(); i != allDeviceIds.end(); ++i) {
                    if (i->isMtw()) {
                        XsDevicePtr mtwDevice = control->device(*i);
                        std::string IDD = mtwDevice->deviceId().toString().toStdString();
                        std::cout << "\n _setup ID: " << IDD << "\n";
                    }
                }*/
                
                
            }catch (std::exception const &ex){
                std::cout << ex.what() << std::endl;
                std::cout << "****ABORT****" << std::endl;
                throw (std::runtime_error(ex.what()));
            }catch (...){
                std::cout << "An unknown fatal error has occured. Aborting." << std::endl;
                std::cout << "****ABORT****" << std::endl;
                throw "****ABORT IMU 2****";
            }

            {
                std::unique_lock<std::mutex> _(_mtx);
                pw.SCOPE.y_axis.minimum = -20.0f;
                pw.SCOPE.y_axis.maximum = 20.0f;
                pw.SCOPE.x_axis.minimum = 0.0f;
                pw.SCOPE.x_axis.maximum = T_exec;
                pw.clearItems();
                pw.addItem("acc 1 x ");
                pw.addItem("acc 1 y ");
                pw.addItem("acc 1 z ");

                pw.addItem("acc 2 x ");
                pw.addItem("acc 2 y ");
                pw.addItem("acc 2 z ");

                pw.addItem("acc 3 x ");
                pw.addItem("acc 3 y ");
                pw.addItem("acc 3 z ");

                pw.addItem("acc 4 x ");
                pw.addItem("acc 4 y ");
                pw.addItem("acc 4 z ");

                pw.addItem("acc 5 x ");
                pw.addItem("acc 5 y ");
                pw.addItem("acc 5 z ");
                
            }

            
        }

        void _cleanup(){
            printf("\nSaliendo de las IMUS");
            try{
                
                if (!wirelessMasterDevice->gotoConfig()){
                    std::ostringstream error;
                    error << "Failed to goto config mode: " << *wirelessMasterDevice;
                    throw std::runtime_error(error.str());
                }
                if (!wirelessMasterDevice->disableRadio()){
                    std::ostringstream error;
                    error << "Failed to disable radio: " << *wirelessMasterDevice;
                    throw std::runtime_error(error.str());
                }
                control->closePort(wirelessMasterPort->portName().toStdString());
            }
            catch (std::exception const& ex) { GUI.addLOG(ex.what()); std::cout << "Error CleanUp Xsens... " << std::endl; throw "****ABORT IMU 2****";
            }
            catch(...){std::cout << "Error CleanUp Xsens... " << std::endl; throw "****ABORT IMU 2****";
            }
        }

        void _firstLoop() {
            using namespace std;

            

           /* XsDeviceIdArray allDeviceIds = control->deviceIds();
            mtwDeviceIds.clear();
            for (XsDeviceIdArray::const_iterator i = allDeviceIds.begin(); i != allDeviceIds.end(); ++i) {
                if (i->isMtw()) {
                    mtwDeviceIds.push_back(*i);
                }
            }
            mtwDevices.clear();
            for (XsDeviceIdArray::const_iterator i = mtwDeviceIds.begin(); i != mtwDeviceIds.end(); ++i) {
                XsDevicePtr mtwDevice = control->device(*i);
                if (mtwDevice != 0) {
                    mtwDevices.push_back(mtwDevice);
                }
                else {
                    throw runtime_error("Failed to create an MTW XsDevice instance");
                }
            }*/


            for (int i = 0; i < sizeof(imu_filters) / sizeof(LowPassFilter2pFloat); i++) {
                imu_filters[i].set_cutoff_frequency(desiredUpdateRate, 16);
                imu_filters[i].reset();
            }
            
            std::cout << "NUMMM >> " << mtwDevices.size() << "\n";
            mtwCallbacks.clear();

            XsDevicePtrArray mtwDevicesOrdered(mtwDevices.size());
            mtwCallbacks.resize(mtwDevices.size());

            for (int i = 0; i < (int)mtwDevices.size(); ++i)
            {
                int idx_unordered(0);
                // loop through mtwDevices to find the index according to the desired imus_names[i] (ordered)
                for (int j = 0; j < (int)mtwDevices.size(); ++j)
                {
                    string check_name = mtwDevices[j]->deviceId().toString().toStdString();
                    if (check_name.compare(imus_names[i]) == 0)
                    {
                        idx_unordered = j;
                        break;
                    }
                }

                mtwDevicesOrdered[i] = mtwDevices[idx_unordered];
                mtwCallbacks[i] = new MtwCallback(i, mtwDevicesOrdered[i]);
                mtwDevicesOrdered[i]->addCallbackHandler(mtwCallbacks[i]);

                string display_name = mtwDevicesOrdered[i]->deviceId().toString().toStdString();
                string imu_placement;

                switch (i)
                {
                case 0:
                    imu_placement = "IMU1 Canela Direita: ";
                    break;
                case 1:
                    imu_placement = "IMU2 Coxa Direita: ";
                    break;
                case 2:
                    imu_placement = "IMU3 Canela Esquerda: ";
                    break;
                case 3:
                    imu_placement = "IMU4 Coxa Esquerda: ";
                    break;
                default:
                    break;
                }
                cout << imu_placement << display_name << "\n";
            }

            eulerData = std::vector<XsEuler>(mtwCallbacks.size());
            accData = std::vector<XsVector>(mtwCallbacks.size());
            gyroData = std::vector<XsVector>(mtwCallbacks.size());
            magData = std::vector<XsVector>(mtwCallbacks.size());

            if (!wirelessMasterDevice->gotoMeasurement()) {
                ostringstream error;
                error << "Failed to goto measurement mode: " << *wirelessMasterDevice;
                throw runtime_error(error.str());
            }
        }

        void _loop(){

            for (size_t i = 0; i < (int)mtwCallbacks.size(); ++i)
            {
                bool newDataAvailable = false;
                if (mtwCallbacks[i] == NULL) continue;

                if (mtwCallbacks[i]->dataAvailable())
                {
                    newDataAvailable = true;
                    XsDataPacket packet = mtwCallbacks[i]->fetchOldestPacket();
                    if (packet.containsCalibratedGyroscopeData())
                        gyroData[i] = packet.calibratedGyroscopeData();

                    if (packet.containsCalibratedAcceleration())
                        accData[i] = packet.calibratedAcceleration();
                }

                if (newDataAvailable) {
                    // Avoid gyroData[i][k] or gyroData[i].at(k) or gyroData[i].value(k)
                    // due to the 'assert' inside these operators on xsvector.h !!!
                    std::vector<XsReal> gyroVector = gyroData[i].toVector();
                    std::vector<XsReal> accVector = accData[i].toVector();
                    
                    if (i == 0) { // IMU1 Canela Direita
                        imus_vals[0] = imu_filters[0].apply(gyroVector[0]);
                        imus_vals[1] = imu_filters[1].apply(gyroVector[1]);
                        imus_vals[2] = imu_filters[2].apply(gyroVector[2]);
                        imus_vals[3] = imu_filters[3].apply( accVector[0]);
                        imus_vals[4] = imu_filters[4].apply( accVector[1]);
                        imus_vals[5] = imu_filters[5].apply( accVector[2]);

                    }
                    if (i == 1) { // IMU2 Coxa Direita
                        imus_vals[6] =  imu_filters[6].apply(gyroVector[0]);
                        imus_vals[7] =  imu_filters[7].apply(gyroVector[1]);
                        imus_vals[8] =  imu_filters[8].apply(gyroVector[2]);
                        imus_vals[9] =  imu_filters[9].apply( accVector[0]);
                        imus_vals[10] = imu_filters[10].apply(accVector[1]);
                        imus_vals[11] = imu_filters[11].apply(accVector[2]);

                    }
                    if (i == 2) { // IMU3 Canela Esquerda
                        imus_vals[12] = imu_filters[12].apply(gyroVector[0]);
                        imus_vals[13] = imu_filters[13].apply(gyroVector[1]);
                        imus_vals[14] = imu_filters[14].apply(gyroVector[2]);
                        imus_vals[15] = imu_filters[15].apply( accVector[0]);
                        imus_vals[16] = imu_filters[16].apply( accVector[1]);
                        imus_vals[17] = imu_filters[17].apply( accVector[2]);

                    }
                    if (i == 3) { // IMU4 Coxa Esquerda
                        imus_vals[18] = imu_filters[18].apply(gyroVector[0]);
                        imus_vals[19] = imu_filters[19].apply(gyroVector[1]);
                        imus_vals[20] = imu_filters[20].apply(gyroVector[2]);
                        imus_vals[21] = imu_filters[21].apply( accVector[0]);
                        imus_vals[22] = imu_filters[22].apply( accVector[1]);
                        imus_vals[23] = imu_filters[23].apply( accVector[2]);
                    }
                    std::unique_lock<std::mutex> _(_mtx);
                    memcpy(data.imus_data, imus_vals, (N_DATA_IMU * sizeof(float)));
                }
            }

            _datalog[time_index][0] = timer->get_current_time_f();
            for (int idx = 0; idx < N_DATA_IMU; idx++) {
                _datalog[time_index][idx + 1] = data.imus_data[idx];
            }

            if (true && (time_index % 4 == 0)) {
                {                    
                    std::unique_lock<std::mutex> _(_mtx);
                    pw.items[0].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 0 + 1]));
                    pw.items[1].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 0 + 2]));
                    pw.items[2].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 0 + 3]));

                    pw.items[3].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 1 + 1]));
                    pw.items[4].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 1 + 2]));
                    pw.items[5].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 1 + 3]));

                    pw.items[6].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 2 + 1]));
                    pw.items[7].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 2 + 2]));
                    pw.items[8].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 2 + 3]));

                    pw.items[9].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 3 + 1]));
                    pw.items[10].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 3 + 2]));
                    pw.items[11].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 3 + 3]));

                    /*pw.items[12].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 4 + 1]));
                    pw.items[13].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 4 + 2]));
                    pw.items[14].data.push_back(ImVec2((float)_datalog[time_index][0], (float)_datalog[time_index][6 * 4 + 3]));
                    */
                }                
            }

        }

        bool cbPlot = false;
        void _updateGUI() {
            ImGui::Begin(_name.c_str());
            
            ImGui::Checkbox("Graph",&cbPlot);
            plotting = cbPlot;
            if (plotting) {
                {
                    std::unique_lock<std::mutex> _(_mtx);
                    pw.show();
                }
            }

            for (int idx = 0; idx < sensorsSxens.size(); idx++) {
                {
                    std::unique_lock<std::mutex> _(_mtx);
                    char name[40];
                    sprintf(name, "Sensor : %d", idx+1);
                    ImGui::Checkbox(name, &sensorsSxens[idx].required);
                    ImGui::SameLine();
                }
            }

            ImGui::End();
        }


        void getData(void * _data){
            {
                std::unique_lock<std::mutex> _(_mtx);
                memcpy(_data,&data,sizeof(fromXsensRead));
            }
        }
};
