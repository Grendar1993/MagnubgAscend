#include "ros/ros.h"
#include "sensor_msgs/PointCoud2.h"
#include "sl/Camera.hpp"
#include <pcl/conversions.h>

#inlcude "pcl/common/common_headers.h"
#include <thread>
#include <mutex>


using namespace sl;
using namespace std;

Camrea zed;
Mat data_cloud;
std::thread zad_callback;
std::mutex mutex_input;
bool stop_signal;
bool has_data;

void startZED();
void run();
void closeZED();
inline float convertColor(float colorIn);

int main(int argc, char **argv){
    ros::init(argc, argv, "zed_point_publisher");
    ros::NodeHandle n;

    ros::Publisher point_pub = n.advertise<sensor_msgs::pointcloud2>("zed_points_pointcloud2",1000);
    ros::Publisher pub = nh.advertise<pcl::PointCloud<pcl::PointXYZRGB> > ("zed_points", 1000);
    ros::Rate loop_rate(10);

    sensor_msgs::PointCoud2 cloud; // construck message

    if (argc > 2) {
        cout << "Only the path of a SVO can be passed in arg" << endl;
        return -1;
    }

    // Set configuration parameters
    InitParameters init_params;
    if (argc == 2)
        init_params.svo_input_filename = argv[1];
    else {
        init_params.camera_resolution = RESOLUTION_VGA;
        init_params.camera_fps = 30;
    }
    init_params.coordinate_units = UNIT_METER;
    init_params.coordinate_system = COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP;
    init_params.depth_mode = DEPTH_MODE_PERFORMANCE;

    ERROR_CODE err = zed.open(init_params);
    if (err != SUCCESS) {
        cout << toString(err) << endl;
        zed.close();
        return 1;
    }

    pcl::PointCloud<pcl::PointXYZRGB>::Ptr p_pcl_point_cloud(new pcl::PointCloud<pcl::PointXYZRGB>);
    p_pcl_point_cloud->points.resize(zed.getResolution().area());


    startZED();


    while (ros::ok()){
        if(mutex_input.try_lock()){
            float *p_data_cloud = data_cloud.getPtr<float>();
            int index = 0;

            for(auto &it : p_pcl_point_cloud->points){
                float X = p_data_cloud[index];
                if (!isValidMeasure(X)) // Checking if it's a valid point
                    it.x = it.y = it.z = it.rgb = 0;
                else {
                    it.x = X;
                    it.y = p_data_cloud[index + 1];
                    it.z = p_data_cloud[index + 2];
                    it.rgb = convertColor(p_data_cloud[index + 3]); // Convert a 32bits float into a pcl .rgb format
                }
                index += 4;
            }
            mutex_input.unlock();
            pub.publish(p_pcl_point_cloud);
            pcl::toROSMsg(*p_pcl_point_cloud, cloud);
            point_pub.publish(cloud); 
            //publish cloud over ros


        } else
            sleep_ms(1);
    }
}


void startZED() {
    // Start the thread for grabbing ZED data
    stop_signal = false;
    has_data = false;
    zed_callback = std::thread(run);

    //Wait for data to be grabbed
    while (!has_data)
        sleep_ms(1);
}


void run() {
    while (!stop_signal) {
        if (zed.grab(SENSING_MODE_STANDARD) == SUCCESS) {
            mutex_input.lock(); // To prevent from data corruption
            zed.retrieveMeasure(data_cloud, MEASURE_XYZRGBA);
            mutex_input.unlock();
            has_data = true;
        } else
            sleep_ms(1);
    }
}


void closeZED() {
    // Stop the thread
    stop_signal = true;
    zed_callback.join();
    zed.close();
}

inline float convertColor(float colorIn) {
    uint32_t color_uint = *(uint32_t *) & colorIn;
    unsigned char *color_uchar = (unsigned char *) &color_uint;
    color_uint = ((uint32_t) color_uchar[0] << 16 | (uint32_t) color_uchar[1] << 8 | (uint32_t) color_uchar[2]);
    return *reinterpret_cast<float *> (&color_uint);
}
